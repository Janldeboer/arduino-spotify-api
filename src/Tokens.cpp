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

void ArduinoSpotify::setRefreshToken(const char *refreshToken)
{
    _refreshToken = refreshToken;
}

bool ArduinoSpotify::refreshAccessToken()
{
    char body[1000];
    sprintf(body, refreshAccessTokensBody, _refreshToken, _clientId, _clientSecret);
    
#ifdef SPOTIFY_DEBUG
    Serial.println(body);
#endif
    
    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, NULL, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
    if (statusCode > 0)
        {
            skipHeaders();
        }
    unsigned long now = millis();
    
#ifdef SPOTIFY_DEBUG
    Serial.print("status Code");
    Serial.println(statusCode);
#endif
    
    bool refreshed = false;
    if (statusCode == 200)
        {
            DynamicJsonDocument doc(1000);
            DeserializationError error = deserializeJson(doc, *client);
            if (!error)
                {
                    sprintf(this->_bearerToken, "Bearer %s", doc["access_token"].as<char *>());
#ifdef SPOTIFY_DEBUG
                    Serial.print("Bearer Token: ");
                    Serial.println(this->_bearerToken);
#endif
                    int tokenTtl = doc["expires_in"];             // Usually 3600 (1 hour)
                    tokenTimeToLiveMs = (tokenTtl * 1000) - 2000; // The 2000 is just to force the token expiry to check if its very close
                    timeTokenRefreshed = now;
                    refreshed = true;
                }
        }
    else
        {
            parseError();
        }
    
    closeClient();
    return refreshed;
}

bool ArduinoSpotify::checkAndRefreshAccessToken()
{
    unsigned long timeSinceLastRefresh = millis() - timeTokenRefreshed;
    if (timeSinceLastRefresh >= tokenTimeToLiveMs)
        {
            Serial.println("Refresh of the Access token is due, doing that now.");
            return refreshAccessToken();
        }
    
    // Token is still valid
    return true;
}

const char *ArduinoSpotify::requestAccessTokens(const char *code, const char *redirectUrl)
{
    
    char body[1000];
    sprintf(body, requestAccessTokensBody, code, redirectUrl, _clientId, _clientSecret);
    
#ifdef SPOTIFY_DEBUG
    Serial.println(body);
#endif
    
    int statusCode = makePostRequest(SPOTIFY_TOKEN_ENDPOINT, NULL, body, "application/x-www-form-urlencoded", SPOTIFY_ACCOUNTS_HOST);
    if (statusCode > 0)
        {
            skipHeaders();
        }
    unsigned long now = millis();
    
#ifdef SPOTIFY_DEBUG
    Serial.print("status Code");
    Serial.println(statusCode);
#endif
    
    if (statusCode == 200)
        {
            DynamicJsonDocument doc(1000);
            DeserializationError error = deserializeJson(doc, *client);
            if (!error)
                {
                    sprintf(this->_bearerToken, "Bearer %s", doc["access_token"].as<char *>());
                    _refreshToken = doc["refresh_token"].as<char *>();
                    int tokenTtl = doc["expires_in"];             // Usually 3600 (1 hour)
                    tokenTimeToLiveMs = (tokenTtl * 1000) - 2000; // The 2000 is just to force the token expiry to check if its very close
                    timeTokenRefreshed = now;
                }
        }
    else
        {
            parseError();
        }
    
    closeClient();
    return _refreshToken;
}