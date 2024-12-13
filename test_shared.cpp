#include <iostream>
#include <memory>
#include <functional>

class test_shared
{
private:
    /* data */
public:
    std::shared_ptr<test_shared> bbb;
    test_shared(/* args */);
    test_shared(std::shared_ptr<test_shared> ccc);
    ~test_shared();
};

test_shared::test_shared(/* args */)
{
}

test_shared::test_shared(std::shared_ptr<test_shared> ccc)
{
    bbb = ccc;
}

test_shared::~test_shared()
{
}

class BlockInfo
{
public:
    // starts from 1
	unsigned num;
	std::shared_ptr<BlockInfo> next;
	BlockInfo()
	{
      num = 0;
	  next = nullptr;
	}
	BlockInfo(unsigned _num)
	{
      num = _num;
	  next = nullptr;
	}

	/**
	 * create BlockInfo
	 * @param _num the number
	 * @param _bp the 'next' pointer
	 */
	BlockInfo(unsigned _num, std::shared_ptr<BlockInfo> _bp)
	{
    	num = _num;
		next = _bp;
	}
    // BlockInfo(unsigned _num)
	// {
    // 	num = _num;
	// 	next = nullptr;
	// }
	~BlockInfo()
	{
		num = 0;
		next = nullptr;
	}
};



int main()
{
    std::shared_ptr<BlockInfo> BlockToWrite = nullptr;

    // if (BlockToWrite != nullptr)
	// {
    //     std::shared_ptr<BlockInfo> p = BlockToWrite;
    //     int i = 0;
    //     while (p != nullptr)
    //     {
    //         std::shared_ptr<BlockInfo> np = p->next;
    //         p.reset();
    //         p = np;
    //         i++;
    //     }
    //     std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite:" << i << std::endl;
    //     BlockToWrite = nullptr;
    // }

    unsigned BaseIndex = 10;

    // for(unsigned i = 10000; i > 0; i--)
    // {
    //     std::shared_ptr<BlockInfo>p = std::make_shared<BlockInfo>(BaseIndex + i - 1, BlockToWrite);
    //     BlockToWrite = p;
    // }

    // if (BlockToWrite != nullptr)
	// {
    //     std::shared_ptr<BlockInfo> p = BlockToWrite;
    //     int i = 0;
    //     while (p != nullptr)
    //     {
    //         std::shared_ptr<BlockInfo> np = p->next;
    //         p.reset();
    //         p = np;
    //         i++;
    //     }
    //     std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite:" << i << std::endl;
    //     BlockToWrite = nullptr;
    // }

    for(unsigned i = 100000; i > 0; i--)
    {
        // std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite:" << i << std::endl;
        std::shared_ptr<BlockInfo>p = std::make_shared<BlockInfo>(BaseIndex + i - 1, BlockToWrite);
        BlockToWrite = p;
    }

    if (BlockToWrite != nullptr)
	{
        std::shared_ptr<BlockInfo> p = BlockToWrite;
        BlockToWrite = nullptr;
        int i = 0;
        while (p != nullptr)
        {
            std::shared_ptr<BlockInfo> np = p->next;
            p.reset();
            p = np;
            i++;
        }
        std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite:" << i << std::endl;
        BlockToWrite = nullptr;
    }

    std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite1111:" << 1 << std::endl;
    BlockToWrite.reset();
    std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite1111:" << 1 << std::endl;
    BlockToWrite = nullptr;
    std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite1111:" << 1 << std::endl;

    // if (BlockToWrite != nullptr)
	// {
    //     std::shared_ptr<BlockInfo> p = BlockToWrite;
    //     int i = 0;
    //     while (p != nullptr)
    //     {
    //         std::shared_ptr<BlockInfo> np = p->next;
    //         p.reset();
    //         p = np;
    //         i++;
    //     }
    //     std::cout << "aaaaaaaaaaaaaaaa1BlockToWrite:" << i << std::endl;
    //     BlockToWrite = nullptr;
    // }

}