/*
 * (C) 2007-2013 Alibaba Group Holding Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version: $Id: tair_mc_client_api.hpp 1245 2013-05-07 15:28:11Z huan.wanghuan@alibaba-inc.com $
 *
 * Authors:
 *   yunhen <huan.wanghuan@alibaba-inc.com>
 *
 */

#include "tair_mc_client_api.hpp"

namespace tair
{
   using namespace std;

   tair_mc_client_api::tair_mc_client_api(): timeout_ms(TAIR_DEFAULT_TIMEOUT)
   {
      diamond_client = new diamond_manager(&controller);
   }
   
   tair_mc_client_api::~tair_mc_client_api()
   {
      if (diamond_client)
      {
         delete diamond_client;
         diamond_client = NULL;
      }
   }

   void tair_mc_client_api::set_net_device_name(const string& net_device_name)
   {
      diamond_client->set_net_device_name(net_device_name);
   }

   bool tair_mc_client_api::startup(const string& config_id)
   {
      return diamond_client->connect(config_id, this);
   }

   void tair_mc_client_api::setup_cache(int area, size_t capacity)
   {
      controller.setup_cache(area, capacity);
   }
   
   void tair_mc_client_api::setup_cache(int area, size_t capacity, uint64_t expire_time)
   {
      controller.setup_cache(area, capacity, expire_time);
   }

   void tair_mc_client_api::close()
   {
      controller.close();
      diamond_client->close();
   }

   const string& tair_mc_client_api::get_config_id()
   {
      return diamond_client->get_config_id();
   }

   const string& tair_mc_client_api::get_data_group_id()
   {
      return diamond_client->get_data_group_id();
   }

   int tair_mc_client_api::put(int area, const data_entry &key, const data_entry &data,
                               int expire, int version, bool fill_cache)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.put(area, key, data, expire, version, fill_cache);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::get(int area, const data_entry &key, data_entry*& data)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.get(area, key, data);
   }

   int tair_mc_client_api:: mget(int area, const vector<data_entry *> &keys,
                                 tair_keyvalue_map& data)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.mget(area, keys, data);
   }

   int tair_mc_client_api::remove(int area, const data_entry &key)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.remove(area, key);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::invalidate(int area, const data_entry &key, bool is_sync)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.invalidate(area, key, is_sync);
   }

   int tair_mc_client_api::prefix_invalidate(int area, const data_entry &key, const data_entry &skey, bool is_sync)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.prefix_invalidate(area, key, skey, is_sync);
   }

   int tair_mc_client_api::hide_by_proxy(int area, const data_entry &key, const char* groupname, bool is_sync)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.hide_by_proxy(area, key, groupname, is_sync);
   }

   int tair_mc_client_api::hide_by_proxy(int area, const data_entry &key, bool is_sync)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.hide_by_proxy(area, key, is_sync);
   }

   int tair_mc_client_api::prefix_hide_by_proxy(int area, const data_entry &pkey, const data_entry &skey, const char *groupname, bool is_sync)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.prefix_hide_by_proxy(area, pkey, skey, groupname, is_sync);
   }

   int tair_mc_client_api::prefix_hide_by_proxy(int area, const data_entry &pkey, const data_entry &skey, bool is_sync)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.prefix_hide_by_proxy(area, pkey, skey, is_sync);
   }

   int tair_mc_client_api::hide(int area, const data_entry &key)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         int code = (*it)->delegate.hide(area, key);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::get_hidden(int area, const data_entry &key, data_entry *&value)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.get_hidden(area, key, value);
   }

   int tair_mc_client_api::prefix_get(int area, const data_entry &pkey, const data_entry &skey, data_entry *&value)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.prefix_get(area, pkey, skey, value);
   }

   int tair_mc_client_api::prefix_gets(int area, const data_entry &pkey, const tair_dataentry_set &skey_set,
                                       tair_keyvalue_map &result_map, key_code_map_t &failed_map)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.prefix_gets(area, pkey, skey_set, result_map, failed_map);
   }

   int tair_mc_client_api::prefix_gets(int area, const data_entry &pkey, const tair_dataentry_vector &skeys,
                                       tair_keyvalue_map &result_map, key_code_map_t &failed_map)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.prefix_gets(area, pkey, skeys, result_map, failed_map);
   }

   int tair_mc_client_api::prefix_put(int area, const data_entry &pkey, const data_entry &skey,
                                      const data_entry &value, int expire, int version)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         int code = (*it)->delegate.prefix_put(area, pkey, skey, value, expire, version);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::prefix_puts(int area, const data_entry &pkey,
                                       const vector<key_value_pack_t*> &skey_value_packs, key_code_map_t &failed_map)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.prefix_puts(area, pkey, skey_value_packs, failed_map);
         if (code != TAIR_RETURN_SUCCESS && code != TAIR_RETURN_PARTIAL_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::prefix_hide(int area, const data_entry &pkey, const data_entry &skey)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         int code = (*it)->delegate.prefix_hide(area, pkey, skey);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::prefix_hides(int area, const data_entry &pkey, const tair_dataentry_set &skey_set,
                                        key_code_map_t &key_code_map)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.prefix_hides(area, pkey, skey_set, key_code_map);
         if (code != TAIR_RETURN_SUCCESS && code != TAIR_RETURN_PARTIAL_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::prefix_get_hidden(int area, const data_entry &pkey, const data_entry &skey, data_entry *&value)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.prefix_get_hidden(area, pkey, skey, value);
   }

   int tair_mc_client_api::prefix_remove(int area, const data_entry &pkey, const data_entry &skey)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.prefix_remove(area, pkey, skey);
         if (code !=  TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::prefix_removes(int area, const data_entry &pkey,
                                          const tair_dataentry_set &skey_set, key_code_map_t &key_code_map)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.prefix_removes(area, pkey, skey_set, key_code_map);
         if (code != TAIR_RETURN_SUCCESS && code != TAIR_RETURN_PARTIAL_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::get_range(int area, const data_entry &pkey, const data_entry &start_key,
                                     const data_entry &end_key, int offset, int limit,
                                     vector<data_entry *> &values, short type)
   {
      tair_client_wrapper_sptr read_client = controller.choose_client_wrapper_for_read();
      return read_client->delegate.get_range(area, pkey, start_key, end_key, offset, limit, values, type);
   }

   int tair_mc_client_api::removes(int area, const tair_dataentry_set &mkey_set, key_code_map_t &key_code_map)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.removes(area, mkey_set, key_code_map);
         if (code != TAIR_RETURN_SUCCESS && code != TAIR_RETURN_PARTIAL_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::hides(int area, const tair_dataentry_set &mkey_set, key_code_map_t &key_code_map)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.hides(area, mkey_set, key_code_map);
         if (code != TAIR_RETURN_SUCCESS && code != TAIR_RETURN_PARTIAL_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::mdelete(int area, const vector<data_entry*> &key)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.mdelete(area, key);
         if (code != TAIR_RETURN_SUCCESS && code != TAIR_RETURN_PARTIAL_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::minvalid(int area, vector<data_entry*> &key)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.minvalid(area, key);
   }

   int tair_mc_client_api::incr(int area, const data_entry& key, int count, int *ret_count, int init_value , int expire)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.incr(area, key, count, ret_count, init_value, expire);
         if (code !=  TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::decr(int area, const data_entry& key, int count, int *ret_count, int init_value , int expire)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.decr(area, key, count, ret_count, init_value, expire);
         if (code !=  TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::add_count(int area, const data_entry &key, int count, int *ret_count, int init_value)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.add_count(area, key, count, ret_count, init_value);
         if (code !=  TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::set_count(int area, const data_entry& key, int count, int expire , int version)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.set_count(area, key, count, expire, version);
         if (code !=  TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::lock(int area, const data_entry& key)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.lock(area, key);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::unlock(int area, const data_entry& key)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.unlock(area, key);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   int tair_mc_client_api::expire(int area, const data_entry& key, int expire)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.expire(area, key, expire);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   void tair_mc_client_api::set_log_level(const char* level)
   {
      TBSYS_LOGGER.setLogLevel(level);
   }

   void tair_mc_client_api::set_log_file(const char* log_file)
   {
      TBSYS_LOGGER.setFileName(log_file);
   }

   void tair_mc_client_api::set_timeout(int timeout)
   {
      timeout_ms = timeout;
      controller.set_timeout_ms(timeout_ms);
   }

   void tair_mc_client_api::set_randread(bool rand_flag)
   {
      controller.set_randread(rand_flag);
   }

#ifdef WITH_COMPRESS
   void tair_mc_client_api::set_compress_type(TAIR_COMPRESS_TYPE type)
   {
      controller.set_compress_type(type);
   }

   void tair_mc_client_api::set_compress_threshold(int threshold)
   {
      controller.set_compress_threshold(threshold);
   }
#endif

   const char* tair_mc_client_api::get_error_msg(int ret)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      return (*write_clients)[0]->delegate.get_error_msg(ret);
   }

   int tair_mc_client_api::mput(int area, const tair_client_kv_map& kvs, int& fail_request, bool compress)
   {
      const client_vector_sptr write_clients = controller.choose_client_wrapper_for_write();
      int code = TAIR_RETURN_SUCCESS;
      for (client_vector::const_iterator it = write_clients->begin(); it != write_clients->end(); ++it)
      {
         code = (*it)->delegate.mput(area, kvs, fail_request, compress);
         if (code != TAIR_RETURN_SUCCESS)
            return code;
      }
      return code;
   }

   void tair_mc_client_api::configOnChanged(const char* dataId, const char* groupId, const char* newContent)
   {
      if (!(dataId && groupId && newContent))
         return ;
      TBSYS_LOG(INFO, "diamond infomation of dataId=%s, groupId=%s changed", dataId, groupId);

      if (diamond_client->get_config_id() != dataId)
         return ;
      // dataId, dataId-GROUP 对应内容发生改变
      if (diamond_client->get_rule_group_id() == groupId)
      {
         if (!diamond_client->update_data_group_id(newContent))
            TBSYS_LOG(ERROR, "update diamond data group id failed");
         return ;
      }
      
      // dataId, real_group_id 对应内容发生改变
      if (diamond_client->get_data_group_id() == groupId)
         if (!diamond_client->update_config(newContent))
            TBSYS_LOG(ERROR, "using config info of diamond dataId=%s, groupId=%s to update tair configserver failed",
                      dataId, groupId);
      return ;
   }
}
