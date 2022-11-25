#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <list>
using namespace std;
int t, max_mem;
struct node
{
    int l, r;
    int idx;
};
list<node> mem;
int cnt = 1;
int main()
{
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    cin >> t >> max_mem;
    string str,str2[10001];
    int ii = 1;
    while(getline(cin,str)){
        str2[ii++] = str;
        if(ii == t) break;
    }
    for (int i = 1; i <= t; i++)
    {
        string tmp = str2[i];
        cout << "tmp" << endl;
        cout << tmp << endl;
        int len = tmp.length();
        stringstream s(tmp);
        string tmp2 = "";
        vector<string> tmp3;
        while (s >> tmp2)
        {
            tmp3.push_back(tmp2);
        }
        len = tmp3.size();
        if (len == 1)
        {
            printf("1");
            int sum = 0;
            int len2 = mem.size();
            if (len2 == 0)
                continue;
            int l = 1;
            for (auto it = mem.begin(); it != mem.end(); it++)
            {
                len2 = it->r - it->l;
                it->l = l;
                it->r = it->l + len2;
                l = it->r + 1;
            }
        }
        else if (len == 2)
        {
            printf("2");
            if (tmp3[0] == "new")
            {
                int para1 = atoi(tmp3[1].c_str());
                int len2 = mem.size();
                int j = 1;
                if (len2 == 0)
                {
                    mem.push_back({1, para1, cnt++});
                    cout << cnt << endl;
                    continue;
                }
                if (mem.cbegin()->l >= para1 + 1)
                {
                    mem.push_front({1, para1, cnt++});
                    cout << cnt << endl;
                    continue;
                }
                for (auto it = mem.begin(); it != mem.end(); j++)
                {
                    if (j == len2)
                    {
                        mem.insert(it, {it->r + 1, it->r + para1, cnt++});
                        cout << cnt << endl;
                        break;
                    }
                    int r = it->r;
                    int len3 = (++it)->l - r - 1;
                    if (len3 < para1)
                    {
                        continue;
                    }
                    else
                    {
                        mem.insert(it, {r + 1, r + para1, cnt++});
                        cout << cnt << endl;
                        break;
                    }
                }
                cout << "NULL" << endl;
            }
            else
            {
                printf("3");
                int para1 = atoi(tmp3[1].c_str());
                if (para1 < 1)
                {
                    cout << "ILLEGAL_OPERATION" << endl;
                    continue;
                }
                for (auto it = mem.begin(); it != mem.end(); it++)
                {
                    if (it->idx == para1)
                    {
                        mem.erase(it);
                        break;
                    }
                }
            }
        }
        else
        {
            int para1 = atoi(tmp3[1].c_str());
            int para2 = atoi(tmp3[2].c_str());
            int len2 = mem.size();
            int j = 1;
            if (len2 == 0)
            {
                mem.push_back({para1, para1 + para2, cnt++});
                cout << cnt << endl;
                continue;
            }
            if (mem.cbegin()->l >= para1 + para2)
            {
                mem.push_back({para1, para1 + para2, cnt++});
                cout << cnt << endl;
                continue;
            }
            for (auto it = mem.begin(); it != mem.end(); j++)
            {
                if (j == len2)
                {
                    if (para1 > it->r)
                    {
                        mem.push_back({it->r + 1, it->r + para2, cnt++});
                        cout << cnt << endl;
                        break;
                    }
                }
                int r = it->r;
                it++;
                int end = it->l;
                if (para1 > r && para1 < end)
                {
                    if (para1 + para2 <= end)
                    {
                        mem.push_back({para1, para1 + para2, cnt++});
                        cout << cnt << endl;
                        break;
                    }
                }
            }
            cout << "NULL" << endl;
        }
    }
    return 0;
}