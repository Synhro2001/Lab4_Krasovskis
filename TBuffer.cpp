#include<Windows.h>
#include <synchapi.h>
#include "TBuffer.h"
#include <cstring>


TBuffer::TBuffer()
{
	this->head = 0; // ��������� *this � ��� ������� ����������� ���������, ���������� ����� �������, ������� �������� ����� ������.
	this->buf[BUF_SIZE] = {};
	this->midProcess = 25;
	this->maxProcess= 50;

	InitializeCriticalSection(&this->cs);  // �������������� ������ ������������ �������

}

TBuffer::~TBuffer() // ���� ������ ����� 1, �� �� ���������� ����� 0, � ���� �� ����� 0, �� �� �������� �������� 1.
{
	DeleteCriticalSection(&this->cs);
}

void TBuffer::Draw(HDC hdc, RECT rect)
{


	HBRUSH blue_brush = CreateSolidBrush(RGB(0, 0, 255)); // ������� CreateSolidBrush ������� ���������� ����� � ��������� �������� ������.
	HPEN red_pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	// CreatePen - ������� CreatePen ������� ���������� ���� � �������� ������, ������� � ������. 
	//������������ ���� ����� ������� � ��������� ���������� � ������������ ��� ��������� ����� � ������.
	HPEN green_pen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

	FillRect(hdc, &rect, blue_brush);


	if (this->head != 0) {
		EnterCriticalSection(&this->cs); 
		// ������� ��������� ����� ������������� �� ��������� ������ ������������ �������.
		//������� ������������, ����� ����������� ������ ��������������� ����� �������������.
		int i = this->head;
		int t = 0;
		//����������� ������� ������ ���� ���������� ������������ Ay � By � ���������� ������� �������������� t � y. 

		int xPrev = 0;
		int UserPrev = 0;
		int KernelPrev = 0;

		while (i != this->head - 1) {

			TBufItem item = this->buf[i];

			int x = t * rect.right / (BUF_SIZE - 1);

			int y = rect.bottom - ((rect.bottom * item.tu) / this->maxProcess);

			int ykernel = rect.bottom - ((rect.bottom * item.tk) / this->maxProcess);



			if (t == 0) {

				SelectObject(hdc, red_pen); // ������� SelectObject �������� ������ � ��������� ��������� ���������� (DC). ����� ������ �������� ���������� ������ ���� �� ����.
				MoveToEx(hdc, x, y, NULL); // ������� MoveToEx ��������� ������� ������� �� ��������� ����� � ��� ������������� ���������� ���������� �������.

				SelectObject(hdc, green_pen);
				MoveToEx(hdc, x, ykernel, NULL);

				UserPrev = y;
				KernelPrev = ykernel;
				xPrev = x;
			}
			else {
				SelectObject(hdc, red_pen);
				MoveToEx(hdc, xPrev, UserPrev, NULL);
				LineTo(hdc, x, y); // ������� lineTo ������ ����� �� ������� ������� �� ��������� �����, �� �� ������� ��.


				SelectObject(hdc, green_pen);
				MoveToEx(hdc, xPrev, KernelPrev, NULL);
				LineTo(hdc, x, ykernel);

				UserPrev = y;
				KernelPrev = ykernel;
				xPrev = x;

			}
			i = (i + 1) % BUF_SIZE;
			t++;

		}


		LeaveCriticalSection(&this->cs);

		DeleteObject(blue_brush);
		DeleteObject(red_pen);
		DeleteObject(green_pen);
	}
}


void TBuffer::Put(TBufItem AItem) {

	EnterCriticalSection(&this->cs);

	this->buf[this->head] = AItem;

	this->head = (this->head + 1) % BUF_SIZE;

	LeaveCriticalSection(&this->cs);
}

void TBuffer::Clear()
{
	memset(this->buf, 0, sizeof(this->buf)); 
	// Memset() - ��� ������� C++. �� �������� ���� ������ � ������ �������� ���������� ���. �� ��������� � ������������ ����� <cstring>.
}

int TBuffer::GetMidProcess() {
	return this->midProcess;
}

int TBuffer::GetMaxProcess() {
	return this->maxProcess;
}

void  TBuffer::IncreaseProcessPercentage() {
	this->maxProcess = this->maxProcess + 10;

	this->midProcess = this->maxProcess / 2;
}

void TBuffer::DecreaseProcessPercentage() {
	this->maxProcess = this->maxProcess - 10;

	this->midProcess = this->maxProcess / 2;
}
