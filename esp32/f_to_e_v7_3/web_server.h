// webserver.h
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>

extern WebServer server;

void setupServer();
void handleHandshake();
void handleControl();
void handleStatus();

#endif
