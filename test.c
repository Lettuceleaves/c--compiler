int rec(int a){
    int i = 0;
    for(i = 0; a < 0; i = i + 1){
        return a + 1;
    }
    print(a);
    int b = rec(a - 1);
    print(b);
    return b + 1;
}

int main(int z) {
    int a = 10;
    rec(a);
}