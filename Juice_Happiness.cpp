/*
Shayan Amouzade   40112126021
Ghazal Molaie     40112126030
*/


#include <utility>
#include <iostream>
#include <algorithm>
#include <cstdio>

using namespace std;
double Juice_Happiness(struct Juice arr[],int N,int amount);
bool cmp(struct Juice a, struct Juice b);

struct Juice
{
    int happiness;
    int volume;
    void set(int h,int v)
    {
        happiness = h;
        volume = v;
    }
};

int main()
{
    int v;
    cout << "Enter max volume: " << endl;
    cin >> v;
    int amount;
    cout << "Enter number of Juices: " << endl;
    cin >> amount;
    
    Juice* arr = new Juice[amount];
    cout << "Enter volume and happiness: " << endl;
    for (int i = 0 ; i < amount ; i++)
    {
        int x,y;
        cin >> x;
        cin >> y;
        arr[i].set(x,y);
    }
    cout << "Maximum happiness: "
        << Juice_Happiness(arr,v,amount);
    delete arr;
    return 0;
}

double Juice_Happiness(struct Juice arr[],int N,int amount)
{
    sort(arr,arr + amount,cmp);
    int curvolume = 0;
    double finalhappiness = 0.0;
    
    for (int i = 0;i<amount;i++) {
        if (curvolume + arr[i].volume <= N) {
            curvolume += arr[i].volume;
            finalhappiness += arr[i].happiness;
        }
        else {
            int remain = N - curvolume;
            finalhappiness += arr[i].happiness
                * ((double)remain
                    / arr[i].volume);
            break;        
        }
    }
    return finalhappiness;
}
bool cmp(struct Juice a,struct Juice b)
{
    double r1 = (double)a.happiness / a.volume;
    double r2 = (double)b.happiness / b.volume;
    return r1 > r2;
}