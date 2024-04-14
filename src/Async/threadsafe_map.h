#pragma once
#include <cstdint>
#include <vector>
#include <list>
#include <shared_mutex>
#include <mutex>
#include <functional>
#include <algorithm>

namespace omp
{
    template < typename Key, typename Value, typename Hash = std::hash<Key> >
    class threadsafe_map
    {
    private:
        // BUCKET //
        // ====== //
        class bucket_type
        {
        public:
            // Types //
            // ===== //
            using bucket_value = std::pair<Key, Value>;
            using bucket_data = std::list<bucket_value>;
            using bucket_iterator = typename bucket_data::iterator;
            using bucket_const_iterator = typename bucket_data::const_iterator;
        private:


            bucket_data m_Data;
            mutable std::shared_mutex m_Mutex;

            bucket_iterator find_entry_for(const Key& key)
            {
                return std::find_if(m_Data.begin(), m_Data.end(), [&](const bucket_value& item)
                {
                    return item.first == key;
                });
            }
            bucket_const_iterator find_entry_for(const Key& key) const
            {
                return std::find_if(m_Data.cbegin(), m_Data.cend(), [&](const bucket_value& item)
                {
                    return item.first == key;
                });
            }


        public:
            Value value_for(const Key& key, const Value& default_value) const
            {
                std::shared_lock<std::shared_mutex> lock(m_Mutex);
                bucket_const_iterator found_entry = find_entry_for(key);
                return (found_entry == m_Data.cend()) ? default_value : found_entry->second;
            }
            void add_or_update_mapping(const Key& key, const Value& value)
            {
                std::unique_lock<std::shared_mutex> lock(m_Mutex);
                bucket_iterator found_entry = find_entry_for(key);
                if (found_entry == m_Data.cend())
                {
                    m_Data.push_back(bucket_value(key, value));
                }
                else
                {
                    found_entry->second = value;
                }
            }
            void remove_mapping(const Key& key)
            {
                std::unique_lock<std::shared_mutex> lock(m_Mutex);
                bucket_iterator found_entry = find_entry_for(key);
                if (found_entry != m_Data.end())
                {
                    m_Data.erase(found_entry);
                }
            }
            void foreach(const std::function<void(bucket_value&)> functor)
            {
                std::unique_lock<std::shared_mutex> lock(m_Mutex);
                for (bucket_value& value : m_Data)
                {
                    functor(value);
                }
            }
        };

        // Hash Map Impl //
        // ============= //
        std::vector<std::unique_ptr<bucket_type>> m_Buckets;
        Hash m_Hasher;

        bucket_type& get_bucket(const Key& key) const
        {
            const std::size_t bucket_index = m_Hasher(key) % m_Buckets.size();
            return *m_Buckets[bucket_index];
        }
        
    public:
        using key_type = Key;
        using mapped_type = Value;
        using hash_type = Hash;

        threadsafe_map(uint32_t num_buckets = 19, const hash_type& hasher = hash_type())
            : m_Buckets(num_buckets)
            , m_Hasher(hasher)
        {
            for (uint32_t index = 0; index < num_buckets; index++)
            {
                m_Buckets[index].reset(new bucket_type);
            }
        }
        threadsafe_map(const threadsafe_map& other) = delete;
        threadsafe_map& operator=(const threadsafe_map& other) = delete;

        // Methods //
        // ======= //
        mapped_type value_for(const key_type& key, const mapped_type& value = mapped_type()) const
        {
            return get_bucket(key).value_for(key, value);
        }

        void add_or_update_mapping(const key_type& key, const mapped_type& value)
        {
            get_bucket(key).add_or_update_mapping(key, value);
        }

        void remove_mapping(const key_type& key)
        {
            get_bucket(key).remove_mapping(key);
        }

        void foreach(const std::function<void(typename bucket_type::bucket_value&)> functor)
        {
            for (auto& bucket : m_Buckets)
            {
                bucket->foreach(functor);
            }
        }

    };
}
