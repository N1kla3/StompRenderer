#pragma once
#include <cstdint>
#include <vector>
#include <list>
#include <shared_mutex>

template < typename Key, typename Value, typename Hash = std::hash<Key> >
class threadsafe_map
{
private:
    // BUCKET //
    // ====== //
    class bucket_type
    {
    private:
        // Types //
        // ===== //
        using bucket_value = std::pair<Key, Value>;
        using bucket_data = std::list<bucket_value>;
        using bucket_iterator = typename bucket_data::iterator;


        bucket_data m_Data;
        mutable std::shared_mutex m_Mutex;

        bucket_iterator find_entry_for(const Key& key) const
        {
            return std::find_if(m_Data.begin(), m_Data.end(), [&](const bucket_value& item)
            {
                return item.first == key;
            });
        }

    public:
        Value value_for(const Key& key, const Value& default_value) const;
        void add_or_update_mapping(const Key& key, const Value& value);
        void remove_mapping(const Key& key);
    };

    // Hash Map Impl //
    // ============= //
    std::vector<std::unique_ptr<bucket_type>> m_Buckets;
    Hash m_hasher;

    bucket_type& get_bucket(const Key& key) const
    {
        const std::size_t bucket_index = m_Hasher(key) % m_Buckets.size();
        return *m_Buckets[bucket_index];
    }
    
public:
    using key_type = Key;
    using mapped_type = Value;
    using hash_type = Hash;

    threadsafe_map(uint32_t num_buckets = 19, const hash_type& = hash_type());
    threadsafe_map(const threadsafe_map& other) = delete;
    threadsafe_map& operator=(const threadsafe_map& other) = delete;

    // Methods //
    // ======= //
    mapped_type value_for(const key_type& key, const mapped_type& value = mapped_type()) const;
    void add_or_update_mapping(const key_type& key, const mapped_type& value);
    void remove_mapping(const key_type& key);
};

