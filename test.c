
int add(int a, int b){
    int c = (1 + 3) * a - b;
    return add(c, 3);
}

int main(int b){
    for(int i = 0; i < 10; i = i + 1){
        b = b + add(i, (1 + 2));
        if(i == 5){
            break;
        }
    }
}