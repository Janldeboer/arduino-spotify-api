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

int ArduinoSpotify::makeRequestWithBody(const char *type, const char *command, bool authorized, const char *accept, const char *body, const char *contentType, const char *host)
{
    client->flush();
    client->setTimeout(SPOTIFY_TIMEOUT);
    if (!client->connect(host, portNumber))
    {
        Serial.println(F("Connection failed"));
        return -1;
    }

    // give the esp a breather
    yield();

    // Send HTTP request
    client->print(type);
    client->print(command);
    client->println(F(" HTTP/1.1"));

    //Headers
    client->print(F("Host: "));
    client->println(host);

    if (accept != NULL)
    {
        client->print(F("Accept: "));
        client->println(accept);
    }
    
    if (contentType != NULL)
    {
        client->print(F("Content-Type: "));
        client->println(contentType);
    }

    if (authorized)
    {
        if (autoTokenRefresh)
        {
            checkAndRefreshAccessToken();
        }
        client->print(F("Authorization: "));
        client->println(_bearerToken);
    }

    client->println(F("Cache-Control: no-cache"));

    if (body != NULL)
    {
        client->print(F("Content-Length: "));
        client->println(strlen(body));
            
        client->println();
            
        client->print(body);
    }

    if (client->println() == 0)
    {
        Serial.println(F("Failed to send request"));
        return -2;
    }

    int statusCode = getHttpStatusCode();
    return statusCode;
}

int ArduinoSpotify::makePutRequest(const char *command, bool authorized, const char *body, const char *contentType, const char *host)
{
    return makeRequestWithBody("PUT ", command, authorized, NULL, body, contentType, host);
}

int ArduinoSpotify::makePostRequest(const char *command, bool authorized, const char *body, const char *contentType, const char *host)
{
    return makeRequestWithBody("POST ", command, authorized, NULL, body, contentType, host);
}

int ArduinoSpotify::makeGetRequest(const char *command, bool authorized, const char *accept, const char *host)
{
    return makeRequestWithBody("GET ", command, authorized, accept, NULL, NULL, host);
}
