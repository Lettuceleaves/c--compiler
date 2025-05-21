int rec(int a){
    int i = 0;
    if(a <= 0) return a;
    print(a);
    int b = rec(a - 1);
    print(b);
    return b + 1;
}

int main(int z) {
    int a = 10;
    rec(a);
}