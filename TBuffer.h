#include <windows.h>
#pragma once
#define BUF_SIZE 100

// tu - процент времени, в течение которого процесс работает в пользовательском режиме.
// tk - Процент времени, в течение которого процесс работает в режиме ядра. 


struct TBufItem { int tu; int tk; };
class TBuffer {
private:
	TBufItem buf[BUF_SIZE]; // массив для хранения данных 
	int head; // самая старая позиция данных 
	CRITICAL_SECTION cs; // Критическая секция
	int midProcess;
	int maxProcess;


public:
	TBuffer(); // konstruktors
	~TBuffer();
	void Draw(HDC hdc, RECT rect);
	void Put(TBufItem AItem); // вставка данных в самое старое местоположение данных 
	void Clear();
	void IncreaseProcessPercentage(); // Инициализирует объект критического раздела.
	void DecreaseProcessPercentage(); // 
	int GetMidProcess();
	int GetMaxProcess();
};