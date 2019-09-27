#include <Windows.h>
#include <stdio.h>
#include <detours.h>

//void TestFunction() {
//	printf("TestFunction Called\n");
//}
//
//void HookingFunction() {
//	printf("before - TestFunction call");
//	TestFunction();
//	printf("after - TestFunction call");
//}
//
//int main(int, char**) {
//	TestFunction();
//	getchar();
//}

void TestFunction() {
	printf("TestFunction Called\n");
}
void(*pfTrueTestFunction)() = TestFunction;
void HookingFunction() {
	printf("before - TestFunction call\n");
	TestFunction();
	//pfTrueTestFunction();
	printf("after - TestFunction call\n");
}

int main(int, char**) {
	DWORD error;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID*)&pfTrueTestFunction, HookingFunction);
	error = DetourTransactionCommit();

	if (error == NO_ERROR) {
		printf("\n");
	}
	else {
		printf("fail to attach\n");
	}
	TestFunction();
	getchar();
}
