// Copyright 2021,2024 Jeisson Hidalgo-Cespedes. ECCI-UCR. CC BY 4.0

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "FactWebApp.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Util.hpp"

static const char* const PREFIX1 = "/fact";
static const char* const PREFIX2 = "/fact?number=";

// TODO(you): Do not duplicate code for other apps, such as GoldbachWebApp.
// Move common code to a base class for Factorization and Goldbach apps

FactWebApp::FactWebApp() {
}

FactWebApp::~FactWebApp() {
}

void FactWebApp::start() {
}

void FactWebApp::stop() {
}

bool FactWebApp::handleHttpRequest(HttpRequest& httpRequest,
    HttpResponse& httpResponse) {
  // If the home page was asked
  // TODO(you): Homepage should be served by a HomeWebApp object
  if (httpRequest.getMethod() == "GET" && httpRequest.getURI() == "/") {
    return this->serveHomepage(httpRequest, httpResponse);
  }
  // If the request starts with "fact/" is for this web app
  if (httpRequest.getURI().rfind(PREFIX1, 0) == 0) {
    return this->serveFactorization(httpRequest, httpResponse);
  }
  // Unrecognized request
  return false;
}

bool FactWebApp::serveHomepage(HttpRequest& httpRequest
  , HttpResponse& httpResponse) {
  (void)httpRequest;
  // TODO(you): Homepage should be served by a HomeWebApp object
  // Build the body of the response
  this->serveHeader(httpResponse);
  httpResponse.body()
    << "  <form method=get action=" << PREFIX1 << ">\n"
    << "    <label for=number>Number</label>\n"
    << "    <input type=text id=number name=number "
              "pattern=\"\\[+\\-]?d+(,[+\\-]?\\d+)*\" required>\n"
    << "    <button type=submit>Factorize</button>\n"
    << "  </form>\n"
    << "</html>\n";
  // Send the response to the client (user agent)
  return httpResponse.send();
}

void FactWebApp::serveHeader(HttpResponse& httpResponse,
    const std::string& title) {
  // Set HTTP response metadata (headers)
  httpResponse.setHeader("Server", "AttoServer v1.0");
  httpResponse.setHeader("Content-type", "text/html; charset=ascii");
  // Serve document header and title
  httpResponse.body() << "<!DOCTYPE html>\n"
    << "<html lang=en>\n"
    << "  <meta charset=utf-8>\n"
    << "  <title>" << title << "</title>\n"
    << "  <style>body {font-family: monospace} .err {color: red}</style>\n"
    << "  <h1>" << title << "</h1>\n";
}

bool FactWebApp::serveFactorization(HttpRequest& httpRequest,
    HttpResponse& httpResponse) {
  // Replace %xx hexadecimal codes by their ASCII symbols
  const std::string& uri = Util::decodeURI(httpRequest.getURI());
  // Build the body of the response
  this->serveHeader(httpResponse);
  httpResponse.body()
    << "  <p>request: " << uri << "</p>\n"
    << "  <ol>\n";
  this->factorizeNumbers(uri, httpResponse);
  httpResponse.body()
    << "  </ol>\n"
    << "  <hr>\n"
    << "  <p><a href=\"/\">Back</a></p>\n"
    << "</html>\n";
  // Send the response to the client (user agent)
  return httpResponse.send();
}

void FactWebApp::factorizeNumbers(const std::string& uri,
    HttpResponse& httpResponse) {
  // Numbers were asked in the form "/fact/123,45,-7899" or "/fact?number=13"
  // Determine the position where numbers start
  size_t numbersStart = std::strlen(PREFIX1);
  if (uri.rfind(PREFIX2, 0) == 0) {
    numbersStart = std::strlen(PREFIX2);
  }
  // TODO(you): Use arbitrary precision for numbers larger than int64_t
  const std::vector<std::string>& texts =
      Util::split(uri.substr(numbersStart), ",", true);
  // For each asked number, provide its prime factorization
  for (size_t index = 0; index < texts.size(); ++index) {
    try {
      // Convert the text to a number. Provide an error message if not
      size_t end = 0;
      const int64_t number = std::stoll(texts[index], &end);
      if (end != texts[index].length()) {
        throw std::runtime_error("invalid number " + texts[index]);
      }
      // Text was converted to number, provide its prime factorization
      // TODO(you): Implement actual factorization by a model object
      // TODO(you): Model objects must not use std::string, only numbers
      httpResponse.body()
        << "    <li>" << number << " = 2<sup>4</sup> * 3 * 5<sup>2</sup>\n";
    } catch (const std::exception& exception) {
      // Text was not a valid number, report an error to user
      httpResponse.body() << "    <li class=err>" << texts[index]
          << ": invalid number\n";
    }
  }
}
