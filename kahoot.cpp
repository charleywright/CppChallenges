// Get websocket token for live kahoot game. Random song (2:15): https://abstract.land/tidal/redirect/111871733

#include <vector>
#include <string>
#include <chrono>
#include <regex>
#include <curl/curl.h>
#include "include/json.hpp"

using json = nlohmann::json;

namespace base64
{
  static std::string encode(std::string in)
  {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in)
    {
      val = (val << 8) + c;
      valb += 8;
      while (valb >= 0)
      {
        out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
        valb -= 6;
      }
    }
    if (valb > -6)
      out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
      out.push_back('=');
    return out;
  }

  static std::string decode(std::string in)
  {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
      T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in)
    {
      if (T[c] == -1)
        break;
      val = (val << 6) + T[c];
      valb += 6;
      if (valb >= 0)
      {
        out.push_back(char((val >> valb) & 0xFF));
        valb -= 8;
      }
    }
    return out;
  }
}

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string clean_eval(std::string in)
{
  std::string out;
  std::string valid_chars = "0123456789()+-/*";
  int pos = 0;
  for (auto c : in)
  {
    if (valid_chars.find(c) != std::string::npos)
      out += c;
  }
  return out;
}

bool is_op(char c) { return (c == '+' || c == '-' || c == '*' || c == '/'); }

int exec(std::string in)
{
  std::vector<std::string> items;
  std::string curr = "";
  for (char i : in)
  {
    if (is_op(i))
    {
      if (curr.length() > 0)
        items.push_back(curr);
      items.push_back(std::string(1, i));
      curr = "";
    }
    else
    {
      curr += i;
    }
  }
  if (curr.length() > 0)
    items.push_back(curr);

  std::vector<char> operators = {'*', '/', '+', '-'};
  for (char _operator : operators)
  {
    int i = 0;
    while (i < items.size())
    {
      if ((items[i][0]) == _operator)
      {
        int prev = std::stoi(items[i - 1]);
        std::vector<std::string>::iterator start;
        switch (_operator)
        {
        case '+':
          prev += std::stoi(items[i + 1]);
          break;
        case '-':
          prev -= std::stoi(items[i + 1]);
          break;
        case '/':
          prev /= std::stoi(items[i + 1]);
          break;
        case '*':
          prev *= std::stoi(items[i + 1]);
          break;
        }
        items[i - 1] = std::to_string(prev);
        start = std::next(items.begin(), i);
        items.erase(start, std::next(start, 2));
      }
      else
        i++;
    }
  }
  return std::stoi(items[items.size() - 1]);
}

int exec_str(std::string str)
{
  std::regex op("\\([0-9*+/-]+\\)", std::regex_constants::ECMAScript);
  std::smatch match;
  while (std::regex_search(str, match, op))
  {
    std::string m = match[0];
    m = m.substr(1, m.length() - 2);
    str = str.replace(str.find(match[0]), match[0].length(), std::to_string(exec(m)));
  }

  return exec(str);
}

std::string get_ws_token(int pin)
{
  int time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  std::string raw_resp, raw_headers, url = std::string("https://kahoot.it/reserve/session/").append(std::to_string(pin)).append("/?").append(std::to_string(time));
  CURL *curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw_resp);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &raw_headers);
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  std::string decode_header = raw_headers.substr(raw_headers.find("x-kahoot-session-token: ") + 24);
  decode_header = decode_header.substr(0, decode_header.find("\n"));
  if (raw_resp == "Not found")
    return "error.INVALID_PIN";
  json parsed = json::parse(raw_resp);
  std::string decode_func = parsed.value("challenge", "");
  if (decode_func.length() == 0)
    return "error.NO_CHALLENGE";
  std::string challenge_code = decode_func.substr(decode_func.find("'") + 1, 100);
  std::string offset_eval = decode_func.substr(decode_func.find("offset") + 6);
  offset_eval = offset_eval.substr(0, offset_eval.find(";"));
  offset_eval = clean_eval(offset_eval);
  int offset = exec_str(offset_eval);
  std::string challenge_token;
  for (int i = 0; i < challenge_code.length(); i++)
  {
    challenge_token += (char)(((((int)challenge_code[i] * i) + offset) % 77) + 48);
  }
  decode_header = base64::decode(decode_header);
  std::string final_token;
  for (int i = 0; i < decode_header.length(); i++)
  {
    int c = (int)decode_header[i];
    int mod = (int)challenge_token[i % challenge_token.length()];
    int decodedChar = c ^ mod;
    final_token += (char)decodedChar;
  }

  std::regex test("^[a-z0-9]+$");
  std::smatch match;
  if (!std::regex_search(final_token, match, test))
  {
    return get_ws_token(pin);
  }
  else
    return final_token;
}

#include <iostream>
int main()
{
  std::string token = get_ws_token(2789871);
  std::cout << token << std::endl;
}