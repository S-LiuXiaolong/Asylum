#include <iostream>
#include <vector>
using namespace std;

class BitMap
{
public:
    BitMap(size_t range)
    {
        //此时多开辟一个空间
        _bits.resize(range / 32 + 1);
    }
    void Set(size_t x)
    {
        int index = x / 32;//确定哪个数据（区间）
        int temp = x % 32;//确定哪个Bit位
        _bits[index] |= (1 << temp);//位操作即可
    }
    void Unset(size_t x)
    {
        int index = x / 32;
        int temp = x % 32;
        _bits[index] &= (0 << temp);//取零
    }
    bool Test(size_t x)
    {
        int index = x / 32;
        int temp = x % 32;
        if (_bits[index]&(1<<temp))
            return 1;
        else
            return 0;
    }

private:
    vector<int> _bits;
};

void TestBitMap()
{
    BitMap bm(200);
    bm.Set(136);
    bm.Set(1);
    cout << bm.Test(32) << endl;
    bm.Unset(136);
    bm.Unset(136);
    cout << bm.Test(136) << endl;
}

int main()
{
    TestBitMap();
    return 0;
}