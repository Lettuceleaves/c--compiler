
int add(int a, int b){
    int c = a + b;
}

int main(int b){
    for(int i = 0; i < 10; i = i + 1){
        b = b + add(i, 3);
        if(i == 5){
            break;
        }
    }
}