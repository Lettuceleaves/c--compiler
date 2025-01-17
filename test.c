int rec(int a){
    int i = 0;
    if(a < 0) return a;
    // print(a);
    int b = rec(a - 1);
    // print(b);
    return b + 1;
}

int main(int z) {
    int a = 5;
    rec(a);
    // ASCII： h: 104 e: 101 l: 108 o: 111 w: 119 r: 114 d: 100 ,: 44 !: 33
    char p = 104;
    print(p);
    p = 101;
    print(p);
    p = 108;
    print(p);
    p = 108;
    print(p);
    p = 111;
    print(p);
    p = 44;
    print(p);
    p = 119;
    print(p);
    p = 111;
    print(p);
    p = 114;
    print(p);
    p = 108;
    print(p);
    p = 100;
    print(p);
    p = 33;
    print(p);
    
}