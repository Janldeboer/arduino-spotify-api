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

bool ArduinoSpotify::play(const char *deviceId)
{
    char command[100] = SPOTIFY_PLAY_ENDPOINT;
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::playAdvanced(char *body, const char *deviceId)
{
    char command[100] = SPOTIFY_PLAY_ENDPOINT;
    return playerControl(command, deviceId, body);
}

bool ArduinoSpotify::pause(const char *deviceId)
{
    char command[100] = SPOTIFY_PAUSE_ENDPOINT;
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::setVolume(int volume, const char *deviceId)
{
    char command[125];
    sprintf(command, SPOTIFY_VOLUME_ENDPOINT, volume);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::toggleShuffle(bool shuffle, const char *deviceId)
{
    char command[125];
    char shuffleState[10];
    if (shuffle)
        {
            strcpy(shuffleState, "true");
        }
    else
        {
            strcpy(shuffleState, "false");
        }
    sprintf(command, SPOTIFY_SHUFFLE_ENDPOINT, shuffleState);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::setRepeatMode(RepeatOptions repeat, const char *deviceId)
{
    char command[125];
    char repeatState[10];
    switch (repeat)
    {
        case repeat_track:
            strcpy(repeatState, "track");
            break;
        case repeat_context:
            strcpy(repeatState, "context");
            break;
        case repeat_off:
            strcpy(repeatState, "off");
            break;
    }
    
    sprintf(command, SPOTIFY_REPEAT_ENDPOINT, repeatState);
    return playerControl(command, deviceId);
}

bool ArduinoSpotify::playerControl(char *command, const char *deviceId, const char *body)
{
    if (deviceId[0] != 0)
        {
            char *questionMarkPointer;
            questionMarkPointer = strchr(command, '?');
            char deviceIdBuff[50];
            if (questionMarkPointer == NULL)
                {
                    sprintf(deviceIdBuff, "?deviceId=%s", deviceId);
                }
            else
                {
                    // params already started
                    sprintf(deviceIdBuff, "&deviceId=%s", deviceId);
                }
            strcat(command, deviceIdBuff);
        }
    
#ifdef SPOTIFY_DEBUG
    Serial.println(command);
    Serial.println(body);
#endif
    
    if (autoTokenRefresh)
        {
            checkAndRefreshAccessToken();
        }
    int statusCode = makePutRequest(command, _bearerToken, body);
    
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::playerNavigate(char *command, const char *deviceId)
{
    if (deviceId[0] != 0)
        {
            char deviceIdBuff[50];
            sprintf(deviceIdBuff, "?deviceId=%s", deviceId);
            strcat(command, deviceIdBuff);
        }
    
#ifdef SPOTIFY_DEBUG
    Serial.println(command);
#endif
    
    if (autoTokenRefresh)
        {
            checkAndRefreshAccessToken();
        }
    int statusCode = makePostRequest(command, _bearerToken);
    
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}

bool ArduinoSpotify::nextTrack(const char *deviceId)
{
    char command[100] = SPOTIFY_NEXT_TRACK_ENDPOINT;
    return playerNavigate(command, deviceId);
}

bool ArduinoSpotify::previousTrack(const char *deviceId)
{
    char command[100] = SPOTIFY_PREVIOUS_TRACK_ENDPOINT;
    return playerNavigate(command, deviceId);
}

bool ArduinoSpotify::seek(int position, const char *deviceId)
{
    char command[100] = SPOTIFY_SEEK_ENDPOINT;
    char tempBuff[100];
    sprintf(tempBuff, "?position_ms=%d", position);
    strcat(command, tempBuff);
    if (deviceId[0] != 0)
        {
            sprintf(tempBuff, "?deviceId=%s", deviceId);
            strcat(command, tempBuff);
        }
    
#ifdef SPOTIFY_DEBUG
    Serial.println(command);
#endif
    
    if (autoTokenRefresh)
        {
            checkAndRefreshAccessToken();
        }
    int statusCode = makePutRequest(command, _bearerToken);
    closeClient();
    //Will return 204 if all went well.
    return statusCode == 204;
}