/*
ArduinoSpotify - An Arduino library to wrap the Spotify API

Copyright (c) 2020  Brian Lough.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "ArduinoSpotify.h"

CurrentlyPlaying ArduinoSpotify::getCurrentlyPlaying(const char *market)
{
    char command[100] = SPOTIFY_CURRENTLY_PLAYING_ENDPOINT;
    if (market[0] != 0)
    {
        char marketBuff[30];
        sprintf(marketBuff, "?market=%s", market);
        strcat(command, marketBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
#endif

    // Get from https://arduinojson.org/v6/assistant/
    const size_t bufferSize = currentlyPlayingBufferSize;
    CurrentlyPlaying currentlyPlaying;
    // This flag will get cleared if all goes well
    currentlyPlaying.error = true;
    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }

    int statusCode = makeGetRequest(command, _bearerToken);
    if (statusCode > 0)
    {
        skipHeaders();
    }

    if (statusCode == 200)
    {
        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            JsonObject item = doc["item"];
            JsonObject firstArtist = item["album"]["artists"][0];

            currentlyPlaying.firstArtistName = (char *)firstArtist["name"].as<char *>();
            currentlyPlaying.firstArtistUri = (char *)firstArtist["uri"].as<char *>();

            currentlyPlaying.albumName = (char *)item["album"]["name"].as<char *>();
            currentlyPlaying.albumUri = (char *)item["album"]["uri"].as<char *>();

            JsonArray images = item["album"]["images"];

            // Images are returned in order of width, so last should be smallest.
            int numImages = images.size();
            int startingIndex = 0;
            if (numImages > SPOTIFY_NUM_ALBUM_IMAGES)
            {
                startingIndex = numImages - SPOTIFY_NUM_ALBUM_IMAGES;
                currentlyPlaying.numImages = SPOTIFY_NUM_ALBUM_IMAGES;
            }
            else
            {
                currentlyPlaying.numImages = numImages;
            }

            for (int i = 0; i < numImages; i++)
            {
                int adjustedIndex = startingIndex + i;
                currentlyPlaying.albumImages[i].height = images[adjustedIndex]["height"].as<int>();
                currentlyPlaying.albumImages[i].width = images[adjustedIndex]["width"].as<int>();
                currentlyPlaying.albumImages[i].url = (char *)images[adjustedIndex]["url"].as<char *>();
            }

            currentlyPlaying.trackName = (char *)item["name"].as<char *>();
            currentlyPlaying.trackUri = (char *)item["uri"].as<char *>();

            currentlyPlaying.isPlaying = doc["is_playing"].as<bool>();

            currentlyPlaying.progressMs = doc["progress_ms"].as<long>();
            currentlyPlaying.duraitonMs = item["duration_ms"].as<long>();

            currentlyPlaying.error = false;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }
    closeClient();
    return currentlyPlaying;
}

AudioFeatures ArduinoSpotify::getAudioFeatures(const char *uri)
{
    AudioFeatures audioFeatures;
    audioFeatures.error = true;

    const char check[] = "spotify:track:";

    char command[200] = SPOTIFY_AUDIO_FEATURES_ENDPOINT;
    strcat(command, "/");

    char id[50];
    strcpy(id, uri);
    int i = 0;
    for (; i < 14; i++)
    {
        if (id[i] != check[i])
        {
#ifdef SPOTIFY_DEBUG
            Serial.println("URI invalid");
#endif
            return audioFeatures;
        }
    }

#ifdef SPOTIFY_DEBUG
    Serial.println("URI valid");
#endif

    strcat(command, id + 14);

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
#endif

    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }

    const size_t bufferSize = audioFeaturesBufferSize;
    int statusCode = makeGetRequest(command, _bearerToken);

    if (statusCode > 0)
    {
        skipHeaders();
    }

    if (statusCode == 200)
    {
        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            audioFeatures.danceability = doc["danceability"].as<float>();
            audioFeatures.energy = doc["energy"].as<float>();
            audioFeatures.key = doc["key"].as<int>();
            audioFeatures.loudness = doc["loudness"].as<float>();
            audioFeatures.mode = doc["mode"].as<int>();
            audioFeatures.speechiness = doc["speechiness"].as<float>();
            audioFeatures.acousticness = doc["acousticness"].as<float>();
            audioFeatures.instrumentalness = doc["instrumentalness"].as<float>();
            audioFeatures.liveness = doc["liveness"].as<float>();
            audioFeatures.valence = doc["valence"].as<float>();
            audioFeatures.tempo = doc["tempo"].as<float>();
            audioFeatures.duration_ms = doc["duration_ms"].as<int>();
            audioFeatures.time_signature = doc["time_signature"].as<int>();

            audioFeatures.error = false;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }

    closeClient();
    return audioFeatures;
}

PlayerDetails ArduinoSpotify::getPlayerDetails(const char *market)
{
    char command[100] = SPOTIFY_PLAYER_ENDPOINT;
    if (market[0] != 0)
    {
        char marketBuff[30];
        sprintf(marketBuff, "?market=%s", market);
        strcat(command, marketBuff);
    }

#ifdef SPOTIFY_DEBUG
    Serial.println(command);
#endif

    // Get from https://arduinojson.org/v6/assistant/
    const size_t bufferSize = playerDetailsBufferSize;
    PlayerDetails playerDetails;
    // This flag will get cleared if all goes well
    playerDetails.error = true;
    if (autoTokenRefresh)
    {
        checkAndRefreshAccessToken();
    }

    int statusCode = makeGetRequest(command, _bearerToken);
    if (statusCode > 0)
    {
        skipHeaders();
    }

    if (statusCode == 200)
    {
        // Allocate DynamicJsonDocument
        DynamicJsonDocument doc(bufferSize);

        // Parse JSON object
        DeserializationError error = deserializeJson(doc, *client);
        if (!error)
        {
            JsonObject device = doc["device"];

            playerDetails.device.id = (char *)device["id"].as<char *>();
            playerDetails.device.name = (char *)device["name"].as<char *>();
            playerDetails.device.type = (char *)device["type"].as<char *>();
            playerDetails.device.isActive = device["is_active"].as<bool>();
            playerDetails.device.isPrivateSession = device["is_private_session"].as<bool>();
            playerDetails.device.isRestricted = device["is_restricted"].as<bool>();
            playerDetails.device.volumePrecent = device["volume_percent"].as<int>();

            playerDetails.progressMs = doc["progress_ms"].as<long>();
            playerDetails.isPlaying = doc["is_playing"].as<bool>();

            playerDetails.shuffleState = doc["shuffle_state"].as<bool>();

            const char *repeat_state = doc["repeat_state"]; // "off"

            if (strncmp(repeat_state, "track", 5) == 0)
            {
                playerDetails.repeateState = repeat_track;
            }
            else if (strncmp(repeat_state, "context", 7) == 0)
            {
                playerDetails.repeateState = repeat_context;
            }
            else
            {
                playerDetails.repeateState = repeat_off;
            }

            playerDetails.error = false;
        }
        else
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
        }
    }
    closeClient();
    return playerDetails;
}

bool ArduinoSpotify::getImage(char *imageUrl, Stream *file)
{
#ifdef SPOTIFY_DEBUG
    Serial.print(F("Parsing image URL: "));
    Serial.println(imageUrl);
#endif

    uint8_t lengthOfString = strlen(imageUrl);

    // We are going to just assume https, that's all I've
    // seen and I can't imagine a company will go back
    // to http

    if (strncmp(imageUrl, "https://", 8) != 0)
    {
        Serial.print(F("Url not in expected format: "));
        Serial.println(imageUrl);
        Serial.println("(expected it to start with \"https://\")");
        return false;
    }

    uint8_t protocolLength = 8;

    char *pathStart = strchr(imageUrl + protocolLength, '/');
    uint8_t pathIndex = pathStart - imageUrl;
    uint8_t pathLength = lengthOfString - pathIndex;
    char path[pathLength + 1];
    strncpy(path, pathStart, pathLength);
    path[pathLength] = '\0';

    uint8_t hostLength = pathIndex - protocolLength;
    char host[hostLength + 1];
    strncpy(host, imageUrl + protocolLength, hostLength);
    host[hostLength] = '\0';

#ifdef SPOTIFY_DEBUG

    Serial.print(F("host: "));
    Serial.println(host);

    Serial.print(F("len:host:"));
    Serial.println(hostLength);

    Serial.print(F("path: "));
    Serial.println(path);

    Serial.print(F("len:path: "));
    Serial.println(strlen(path));
#endif

    bool status = false;
    int statusCode = makeGetRequest(path, NULL, "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", host);
#ifdef SPOTIFY_DEBUG
    Serial.print(F("statusCode: "));
    Serial.println(statusCode);
#endif
    if (statusCode == 200)
    {
        int totalLength = getContentLength();
#ifdef SPOTIFY_DEBUG
        Serial.print(F("file length: "));
        Serial.println(totalLength);
#endif
        if (totalLength > 0)
        {
            skipHeaders(false);
            int remaining = totalLength;
            // This section of code is inspired but the "Web_Jpg"
            // example of TJpg_Decoder
            // https://github.com/Bodmer/TJpg_Decoder
            // -----------
            uint8_t buff[128] = {0};
            while (client->connected() && (remaining > 0 || remaining == -1))
            { // Get available data size
                size_t size = client->available();

                if (size)
                {
                    // Read up to 128 bytes
                    int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                    // Write it to file
                    file->write(buff, c);

                    // Calculate remaining bytes
                    if (remaining > 0)
                    {
                        remaining -= c;
                    }
                }
                yield();
            }
            // ---------
#ifdef SPOTIFY_DEBUG
            Serial.println(F("Finished getting image"));
#endif
            // probably?!
            status = true;
        }
    }

    closeClient();

    return status;
}

