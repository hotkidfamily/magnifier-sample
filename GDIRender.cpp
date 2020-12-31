#include "GDIRender.h"

CGDIRender::CGDIRender()
{
}

CGDIRender::~CGDIRender()
{
}


void CGDIRender::reset()
{
	if (_hWnd)
	{
		if(_hDC)
			ReleaseDC(_hWnd, _hDC);

		_hDC = nullptr;
	}

	if(_hBitmap){
		::DeleteObject(_hBitmap);
		_hBitmap = nullptr;
	}

	if (_hMemDC) {
		::DeleteObject(_hMemDC);
		_hMemDC = nullptr;
	}
}

void CGDIRender::render(HWND hWnd, std::unique_ptr<DesktopFrame> &frame)
{
	auto srcW = frame->size().width();
	auto srcH = frame->size().height();
	auto data = frame->data();
	auto dpi = GetDpiForWindow(hWnd);
	auto sysDpi = GetDpiForSystem();

	int vW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int vH = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	int vLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int vTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

	int pW = GetSystemMetrics(SM_CXSCREEN);
	int pH = GetSystemMetrics(SM_CYSCREEN);

	char buff[256];
	sprintf_s(buff, 256, "%dx%d,Win-DPI %d,Sys-DPI %d,Vrect(%dx%d,%dx%d), pRi(0x0, %dx%d)", srcW, srcH, dpi, sysDpi, vLeft, vTop, vW, vH,  pW, pH);

	RECT rect;
	GetWindowRect(hWnd, &rect);
	RECT rect2;
	GetClientRect(_hWnd, &rect2);

	if (_Size.width() != srcW || _Size.height() != srcH || hWnd != _hWnd) {
		reset();

		_Size.set(srcW, srcH);

		_hWnd = hWnd;
		_hDC = GetDC(_hWnd);

		_hMemDC = ::CreateCompatibleDC(_hDC);
		ConstructBMPHeader(srcW, srcH, pix_fmt::RGBA, _bmpHeader);
		_hBitmap = ::CreateDIBSection(_hMemDC, (BITMAPINFO*)&_bmpHeader, DIB_RGB_COLORS, (void**)data, NULL, 0);

		::SelectObject(_hMemDC, _hBitmap);
	}

	auto w = rect.right - rect.left;
	auto h = rect.bottom - rect.top;

	::SetDIBitsToDevice(_hMemDC, 0, 0, srcW, srcH, 0, 0, 0, srcH, data, (BITMAPINFO*)&_bmpHeader, DIB_RGB_COLORS);
	::SetStretchBltMode(_hDC, COLORONCOLOR);

	//::BitBlt(_hDC, 0, 0, w, h, _hMemDC, 0, 0, SRCCOPY);
	::StretchBlt(_hDC, 0, 0, w, h-100, _hMemDC, 0, srcH-1, srcW, -srcH, SRCCOPY);
	::DrawTextA(_hDC, buff, -1, &rect2, DT_SINGLELINE | DT_NOCLIP);
}

void CGDIRender::ConstructBMPHeader(int width, int height, pix_fmt format, BITMAPINFOHEADER& bitmapHeader)
{
	int widthStep = 0;
	bitmapHeader.biSize = (DWORD)sizeof(BITMAPINFOHEADER);// header size
	bitmapHeader.biWidth = width;
	bitmapHeader.biHeight = height;
	bitmapHeader.biPlanes = 1;
	bitmapHeader.biCompression = BI_RGB;
	bitmapHeader.biXPelsPerMeter = 0;
	bitmapHeader.biYPelsPerMeter = 0;
	bitmapHeader.biClrUsed = 0;
	bitmapHeader.biClrImportant = 0;
	switch (format)
	{
	case pix_fmt::RGB24:
	case pix_fmt::BGR24:
		widthStep = (((width * 24) + 31) & (~31)) / 8;
		bitmapHeader.biBitCount = 24;     // RGB encoded, 24 bit
		bitmapHeader.biSizeImage = widthStep * height;
		break;
	case pix_fmt::BGRA:
	case pix_fmt::RGBA:
		widthStep = (((width * 32) + 31) & (~31)) / 8;
		bitmapHeader.biBitCount = 32;
		bitmapHeader.biSizeImage = widthStep * height;
		break;
	}
}

