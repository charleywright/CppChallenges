// Parses a magnet: URI. Random song: https://abstract.land/tidal/redirect/158042066
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <regex>
#include <iostream>

namespace url
{
  std::string encode(std::string str)
  {
    std::string new_str = "";
    char c;
    int ic;
    const char *chars = str.c_str();
    char buf_hex[10];
    int len = strlen(chars);

    for (int i = 0; i < len; i++)
    {
      c = chars[i];
      ic = c;
      if (c == ' ')
        new_str += '+';
      else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        new_str += c;
      else
      {
        sprintf(buf_hex, "%X", c);
        if (ic < 16)
          new_str += "%0";
        else
          new_str += "%";
        new_str += buf_hex;
      }
    }
    return new_str;
  }

  std::string decode(std::string str)
  {
    std::string ret;
    char ch;
    int i, ii, len = str.length();

    for (i = 0; i < len; i++)
    {
      if (str[i] != '%')
      {
        if (str[i] == '+')
          ret += ' ';
        else
          ret += str[i];
      }
      else
      {
        sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
        ch = static_cast<char>(ii);
        ret += ch;
        i = i + 2;
      }
    }
    return ret;
  }
}

namespace tracker
{
  // See https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
  addrinfo *get_addr_info_from_tracker(const char *_host, const char *_port)
  {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    s = getaddrinfo(_host, _port, &hints, &result);
    if (s != 0)
    {
      return NULL;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket(rp->ai_family, rp->ai_socktype,
                   rp->ai_protocol);
      if (sfd == -1)
        continue;

      if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
        break;

      close(sfd);
    }

    return rp;
  }

  struct Tracker
  {
    std::string ip;
    std::string host;
    std::string path;
    unsigned short port;
    addrinfo *addr_info;
    Tracker(std::string url)
    {
      if (url.substr(6).find(":") != -1)
      {
        port = std::stoi(url.substr(url.substr(6).find(":") + 7, url.substr(6).find("/") - url.substr(6).find(":") - 1));
      }
      else
        port = 161;
      std::size_t host_start = url.find("://") + 3;
      host = url.substr(host_start, url.find(std::to_string(port)) - host_start - 1);
      path = url.substr(url.find(std::to_string(port)) + std::to_string(port).length());
      if (path.length() == 0)
        path = "/";
      addr_info = get_addr_info_from_tracker(host.c_str(), std::to_string(port).c_str());
      if (addr_info != NULL)
      {
        struct in_addr addr;
        addr.s_addr = ((struct sockaddr_in *)(addr_info->ai_addr))->sin_addr.s_addr;
        ip = inet_ntoa(addr);
      }
      else
        ip = "";
    }
  };
}

namespace magnet
{
  std::string get_info_hash(const std::string magnet)
  {
    std::regex xt_expr("xt=urn:[a-zA-Z0-9:]+");
    std::smatch xt_match;
    std::regex_search(magnet, xt_match, xt_expr);
    if (xt_match.size() == 0)
    {
      std::cout << "\nInvalid URI" << std::endl;
      exit(1);
    }
    return xt_match.str().substr(xt_match.str().find_last_of(":") + 1);
  }

  std::string get_name(const std::string magnet)
  {
    std::regex xt_expr("dn=[a-zA-Z0-9+\\-%.]+");
    std::smatch xt_match;
    std::regex_search(magnet, xt_match, xt_expr);
    if (xt_match.size() == 0)
    {
      return "No Name";
    }

    return url::decode(xt_match.str().substr(3));
  }

  std::vector<tracker::Tracker> get_trackers(std::string magnet)
  {
    std::vector<tracker::Tracker> trackers;
    bool match = magnet.find("tr=") != -1;
    while (match)
    {
      match = false;
      int start = magnet.find("tr=");
      int end = magnet.substr(start).find("&");
      std::string tracker;
      if (end == -1)
      {
        tracker = magnet.substr(start + 3);
      }
      else
        tracker = magnet.substr(start + 3, end - 3);

      tracker::Tracker t = tracker::Tracker(url::decode(tracker));
      if (t.addr_info == NULL)
        continue;
      trackers.push_back(t);

      if (end != -1)
      {
        magnet = magnet.substr(start + end);
        match = true;
      }
    }

    return trackers;
  }
}

std::string get_spaces(std::string input, int length)
{
  std::string output = input;
  for (int i = 0; i < (length - input.length()); i++)
  {
    output += " ";
  }
  return output;
}
int main()
{
  std::string _magnet = "magnet:?xt=urn:btih:0AFF4A1A75E0857ADD1B77B6E0D0828D487A43A6&dn=OBS+Studio+24.0.3&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.openbittorrent.com%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.dler.org%3A6969%2Fannounce&tr=udp%3A%2F%2Fopentracker.i2p.rocks%3A6969%2Fannounce&tr=udp%3A%2F%2F47.ip-51-68-199.eu%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.internetwarriors.net%3A1337%2Fannounce&tr=udp%3A%2F%2F9.rarbg.to%3A2920%2Fannounce&tr=udp%3A%2F%2Ftracker.pirateparty.gr%3A6969%2Fannounce&tr=udp%3A%2F%2Ftracker.cyberia.is%3A6969%2Fannounce";
  std::string _infohash = magnet::get_info_hash(_magnet);
  std::string _name = magnet::get_name(_magnet);
  std::vector<tracker::Tracker> _trackers = magnet::get_trackers(_magnet);

  std::cout
      << "Name: " << _name << "\nInfo Hash: " << _infohash << "\n\nTrackers:" << std::endl;
  for (tracker::Tracker t : _trackers)
  {
    std::cout << "Host: " << get_spaces(t.host, 35) << "IP: " << get_spaces(t.ip, 20) << "Port: " << get_spaces(std::to_string(t.port), 5) << "Path: " << t.path << std::endl;
  }
}