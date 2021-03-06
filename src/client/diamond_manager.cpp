/*
 * (C) 2007-2013 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version: $Id: tair_mc_client_api.hpp 1245 2013-05-13 11:32:45Z huan.wanghuan@alibaba-inc.com $
 *
 * Authors:
 *   yunhen <huan.wanghuan@alibaba-inc.com>
 *
 */

#include "diamond_manager.hpp"

#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>

namespace tair{

   using std::string;

   diamond_manager::diamond_manager(cluster_controller* a_controller)
   {
      listener = NULL;
      config["diamond.invoke.timeout"] = "30";    // timeout，单位秒
      config["diamond.polling.interval"] = "5000000"; // 轮询间隔，单位纳秒，5ms = 5*1000*1000ns
//    容灾目录配置，默认是 ${HOME}/diamond
//    config["diamond.localConfigDir"] = ...
      diamond_client = new DIAMOND::DiamondClient(&config);
      controller = a_controller;
   }

   diamond_manager::~diamond_manager()
   {
      if (diamond_client) {
         delete diamond_client;
         diamond_client = NULL;
      }
   }

   void diamond_manager::set_net_device_name(const string& a_net_device_name) {
      net_device_name = a_net_device_name;
   }

   bool diamond_manager::connect(const string& a_config_id, DIAMOND::SubscriberListener *a_listener)
   {
      listener = a_listener;
      config_id = a_config_id;
      rule_group_id = config_id + "-GROUP";

      string md5;
      string rule_group_content;
      string error;
      string bad_content("config data not exist");
      if((!diamond_client->getConfig(config_id.c_str(), rule_group_id.c_str(), md5 ,rule_group_content, error))
         || (rule_group_content.compare(0, bad_content.size(), bad_content) == 0))
      {
         TBSYS_LOG(ERROR, "can't get config info from the diamond dataId=%s, groupId=%s: %s",
                   config_id.c_str(), rule_group_id.c_str(), error.c_str());
         return false;
      }
      if (!update_data_group_id(rule_group_content))
         return false;
      if (listener)
         diamond_client->registerListener(config_id.c_str(), rule_group_id.c_str(), listener, md5);
      return true;
   }

   void diamond_manager::close()
   {
      diamond_client->shutdownListen();
   }

   const string& diamond_manager::get_config_id()
   {
      return config_id;
   }

   const string& diamond_manager::get_rule_group_id()
   {
      return rule_group_id;
   }

   const string& diamond_manager::get_data_group_id()
   {
      return data_group_id;
   }

   bool diamond_manager::update_data_group_id(const string& rule_group_content)
   {
      vector<string> rule_ip, rule_dest;
      get_route_rule(rule_group_content, rule_ip, rule_dest);
      vector<string> local_ip;
      if (!get_local_ip(local_ip))
      {
         TBSYS_LOG(ERROR, "get local ip failed");
         return false;
      }
      string new_data_group_id;
      if (!search_data_group(local_ip, rule_ip, rule_dest, new_data_group_id))
      {
         TBSYS_LOG(ERROR, "get diamond data group id failed, you ip doesn't match any group rule");
         return false;
      }

      if (new_data_group_id.empty())
      {
         TBSYS_LOG(ERROR, "get diamond data group id failed, you rule group config is invalid");
         return false;
      }
      //rule group改变了，本机 对应的data group未变的话，直接返回
      if (!(new_data_group_id != data_group_id))
      {
         TBSYS_LOG(INFO, "data group id doesn't change");
         return true;
      }

      string md5, data_group_content, error;
      string bad_content("config data not exist");
      if ((!diamond_client->getConfig(config_id.c_str(), new_data_group_id.c_str(), md5, data_group_content, error))
          || (data_group_content.compare(0, bad_content.size(), bad_content) == 0))
      {
         TBSYS_LOG(ERROR, "can't get config info from the diamond dataId=%s, groupId=%s: %s",
                   config_id.c_str(), new_data_group_id.c_str(), error.c_str());
         return false;
      }
         
      if (!update_config(data_group_content))
      {
         TBSYS_LOG(ERROR, "using config info of diamond dataId=%s, groupId=%s to update tair configserver failed",
                   config_id.c_str(), new_data_group_id.c_str());
         return false;
      }

      if (listener)
      {
         if(!data_group_id.empty())
            diamond_client->removeListener(config_id.c_str(), data_group_id.c_str());
         diamond_client->registerListener(config_id.c_str(), new_data_group_id.c_str(), listener, md5);
      }
      
      data_group_id = new_data_group_id;

      return true;
   }

   bool diamond_manager::update_config(const string& json)
   {
      if (!controller)
      {
         TBSYS_LOG(ERROR, "cluster_controller == NULL");
         return false;
      }
      return controller->update_config(json);
   }

   void diamond_manager::get_route_rule(const string& group_rule, vector<string>& rule_ip, vector<string>& rule_dest)
   {
      istringstream group_rule_in(group_rule);
      while(true)
      {
         string one_line;
         if (!getline(group_rule_in, one_line))
            break;
         
         string::size_type pos = one_line.find_first_of('=');
         if (string::npos == pos)
            continue;

         string ip(one_line, 0, pos);
         string dest(one_line, pos + 1);
         
         string::size_type begin_ip = ip.find_first_not_of(" \r\t");
         if (string::npos == begin_ip)
            continue;
         string::size_type end_ip = ip.find_last_not_of(" \r\t");

         
         string::size_type begin_dest = dest.find_first_not_of(" \r\t");
         if (string::npos == begin_dest)
            continue;
         string::size_type end_dest = dest.find_last_not_of(" \r\t");
         
         rule_ip.push_back(ip.substr(begin_ip, end_ip - begin_ip + 1));
         rule_dest.push_back(dest.substr(begin_dest, end_dest - begin_dest + 1));
      }
   }

   bool diamond_manager::get_local_ip(vector<string>& ip)
   {
      struct ifaddrs *ifaddr, *ifa;
      int family, s;
      char host[NI_MAXHOST];

      if (getifaddrs(&ifaddr) == -1)
         return false;
      /* Walk through linked list, maintaining head pointer so we
         can free list later */
      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
      {
         if (!ifa->ifa_addr)
            continue;
         family = ifa->ifa_addr->sa_family;
         if (family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0)
         {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s)
               continue;
            //若设置了net_device_name, 只取net_device_name对应的ip; 否则取所有ipv4的ip（回环除外）
            if (net_device_name.empty())
               ip.push_back(host);
            else
            {
               if(ifa->ifa_name == net_device_name)
               {
                  ip.push_back(host);
                  break;
               }
            }
         }
      } // end of for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
      freeifaddrs(ifaddr);
      return true;
   }

   // 对于多个本地ip，若与分组规则都有匹配，选匹配分组最靠前的。
   // 例如，ipA 匹配了 第1个分组，ipB 匹配了 第4个分组，则选择 第1个分组规则。
   bool diamond_manager::search_data_group(const vector<string>& local_ip, const vector<string>& rule_ip,
                                           const vector<string>& rule_dest, string& data_group_id)
   {
      vector<string>::size_type min = rule_ip.size();
      for (vector<string>::const_iterator ip_iter = local_ip.begin(); ip_iter != local_ip.end(); ++ip_iter)
      {
         vector<string>::size_type number = 0;
         for(vector<string>::const_iterator rule_ip_iter = rule_ip.begin(); rule_ip_iter != rule_ip.end(); ++rule_ip_iter)
         {
            if (ip_iter->compare(0, rule_ip_iter->size(), *rule_ip_iter) == 0)
            {
               if (number < min)
                  min = number;
               break;
            }
            ++number;
         }
      }// end of for (vector<string>::const_iterator ip_iter; ip_iter != local_ip.end(); ++ip_iter)
      if (rule_ip.size() == min)
         return false;
      data_group_id = rule_dest[min];
      return true;
   }
}
