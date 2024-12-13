/*
 * @Author: hexuejiang
 * @Date: 2024-11-5 14:52:50
 * @LastEditTime: 2024-11-13 14:50:20
 * @LastEditors: hexuejiang 1632802996@qq.com
 * @Description: manage entry memory block by enqual length block
 */
#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "IVArray/IVEntry.h"
#include "ISArray/ISEntry.h"

#define ENTRY_BLOCK_SIZE 102400

template <typename Ttype>
class EntryBlock
{
    private:
    std::shared_ptr<Ttype []> array_;

    public:
    EntryBlock()
    {
        array_ = nullptr;
    }

    ~EntryBlock(){}

    void allocEntry()
    {
        std::shared_ptr<Ttype []> new_ptr(new Ttype[ENTRY_BLOCK_SIZE], [](Ttype *p)
        {
            // SLOG_TRACE("delete allocEntry");
            delete [] p;
        });
        array_ = new_ptr;
    }

    Ttype* key(int id)
    {
        return &array_[id];
    }
};


template <typename Ttype>
class EntryBlockList
{
    private:
    std::vector<std::shared_ptr<EntryBlock<Ttype>>> vBlock_;
    
    size_t block_num_;
    public:
    EntryBlockList()
    {
        block_num_ = 0;
    }
    ~EntryBlockList(){}

    void allocBlock(int key)
    {
        int belong_block = getNeedBlock(key);
        while (belong_block > block_num_)
        {
            // SLOG_TRACE("add allocBlock key:" << key << "cur num:" << block_num_ << " ,need num:" << belong_block);
            std::shared_ptr<EntryBlock<Ttype>> new_ptr = std::make_shared<EntryBlock<Ttype>>();
            new_ptr->allocEntry();
            vBlock_.push_back(new_ptr);
            block_num_++;
        }
    }

    Ttype* operator[](int key)
    {
        if (key >= block_num_*ENTRY_BLOCK_SIZE)
        {
            allocBlock(key);
        }
        return vBlock_[blockId(key)]->key(entryId(key));
    }

    int getNeedBlock(int key)const
    {
        if (key < 0)
        {
            SLOG_ERROR("blockKey error:" << key);
            return 0;
        }
        return key/ENTRY_BLOCK_SIZE + 1;
    }

    int blockId(int key)const
    {
        if (key < 0)
        {
            SLOG_ERROR("blockKey error:" << key);
            return 0;
        }
        return key/ENTRY_BLOCK_SIZE;
    }

    int entryId(int key)const
    {
        return key%ENTRY_BLOCK_SIZE;
    }

    bool empty()const{ return block_num_ == 0; }

    unsigned getEntryNum()const
    {
        return block_num_ * ENTRY_BLOCK_SIZE;
    }

    void printStr(int key, char *str, unsigned long len)
    {
        std::string print;
        for (unsigned long i = 0; i < len; i++)
            print += str[i];
        SLOG_TRACE("printStr key:" << key << "  ,str:" << print);
    }

    void printStr(int key, char *str, unsigned len)
    {
        std::string print;
        for (unsigned i = 0; i < len; i++)
            print += str[i];
        SLOG_TRACE("printStr key:" << key << "  ,str:" << print);
    }
};