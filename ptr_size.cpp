// Fails to compiler under 64 bit mode

template<unsigned long Size> struct TestSize; // #1
template<> struct TestSize<4> {}; // #2

int main()
{
    TestSize<sizeof(void*)>();
    return 0;
}