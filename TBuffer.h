#include <windows.h>
#pragma once
#define BUF_SIZE 100

// tu - ������� �������, � ������� �������� ������� �������� � ���������������� ������.
// tk - ������� �������, � ������� �������� ������� �������� � ������ ����. 


struct TBufItem { int tu; int tk; };
class TBuffer {
private:
	TBufItem buf[BUF_SIZE]; // ������ ��� �������� ������ 
	int head; // ����� ������ ������� ������ 
	CRITICAL_SECTION cs; // ����������� ������
	int midProcess;
	int maxProcess;


public:
	TBuffer(); // konstruktors
	~TBuffer();
	void Draw(HDC hdc, RECT rect);
	void Put(TBufItem AItem); // ������� ������ � ����� ������ �������������� ������ 
	void Clear();
	void IncreaseProcessPercentage(); // �������������� ������ ������������ �������.
	void DecreaseProcessPercentage(); // 
	int GetMidProcess();
	int GetMaxProcess();
};