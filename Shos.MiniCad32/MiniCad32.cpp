// SDKDDKVer.h をインクルードすると、利用できる最も上位の Windows プラットフォームが定義されます。
// 以前の Windows プラットフォーム用にアプリケーションをビルドする場合は、WinSDKVer.h をインクルードし、
// SDKDDKVer.h をインクルードする前に、サポート対象とするプラットフォームを示すように _WIN32_WINNT マクロを設定します。
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN // Windows ヘッダーから使用されていない部分を除外します。
#include <windows.h>
#include <windowsx.h>

#include <memory>
#include <string>
#include <sstream>
#include <exception>
#include <vector>
#include <algorithm>
using namespace std;

#include <cassert>
#include <tchar.h>

#include "resource.h"

namespace Shos {
namespace MiniCad {

typedef basic_string<_TCHAR, char_traits<_TCHAR>, allocator<_TCHAR>> tstring;
typedef basic_stringstream<_TCHAR, char_traits<_TCHAR>, allocator<_TCHAR>> tstringstream;

namespace Diagnostics {
#ifdef _DEBUG
#ifndef DEBUG_NEW

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__ , __LINE__)
#define new DEBUG_NEW

class MemoryLeakDetector
{
    static MemoryLeakDetector instance;

    MemoryLeakDetector()
    {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    }
};

MemoryLeakDetector MemoryLeakDetector::instance;

#endif
#endif  // _DEBUG

class Debug
{
public:

    static void Assert(bool expression)
    {
#ifdef _DEBUG
        assert(expression);
#endif  // _DEBUG
    }

    static void Trace(tstring messageText)
    {
#ifdef _DEBUG
#if _UNICODE
        _RPTW0(_CRT_WARN, messageText.c_str());
#else
        _RPT0(_CRT_WARN, messageText.c_str());
#endif
#endif  // _DEBUG
    }
};

} // namespace Diagnostics

namespace Common {

class Observer
{
public:
    virtual void OnUpdate(void* data)
    {}
};

class Observable
{
    vector<Observer*> observers;

public:
    void AddObserver(Observer& observer)
    {
        observers.push_back(&observer);
    }

protected:
    void Update(void* data)
    {
        for (auto observer : observers)
            observer->OnUpdate(data);
    }
};

class Uncopyable
{
public:
    Uncopyable()
    {}

private:
    Uncopyable(const Uncopyable&)
    {}
    void operator =(const Uncopyable&)
    {}
};

class Math
{
public:
	template <class T>
	static T Min(T value1, T value2)
	{
		return value1 < value2 ? value1 : value2;
	}

	template <class T>
	static T Max(T value1, T value2)
	{
		return value1 > value2 ? value1 : value2;
	}

    template <class T>
    static T Average(T value1, T value2)
    {
        return (value1 + value2) / 2;
    }

    template <class T>
    static T Square(T x)
    {
        return x * x;
    }

    static long Round(double value)
    {
        return static_cast<long>(::floor(value + 0.5));
    }
};

//class Utility
//{
//public:
//    template <class T>
//    static void Swap(unique_ptr<T>& pointer1, unique_ptr<T>& pointer2)
//    {
//        const auto temporary = pointer1.release();
//        pointer1.reset(pointer2.release());
//        pointer2.reset(temporary);
//    }
//};

} // namespace Common

namespace Windows {
using namespace Diagnostics;
using namespace Common;

struct CSize : public SIZE
{
    CSize(int cx = 0, int cy = 0)
    {
        this->cx = cx;
        this->cy = cy;
    }

    CSize(const SIZE& size)
    {
        this->cx = size.cx;
        this->cy = size.cy;
    }

    CSize operator +(const SIZE& size) const
    {
        return CSize(cx + size.cx, cy + size.cy);
    }

    CSize operator -(const SIZE& size) const
    {
        return CSize(cx - size.cx, cy - size.cy);
    }

    CSize operator /(long dividor) const
    {
        return CSize(cx / dividor, cy / dividor);
    }

    long Absolute() const
    {
        return Math::Round(::sqrt(Math::Square(cx) + Math::Square(cy)));
    }

    long GetInnerProduct(CSize size)
    {
        return cx * size.cx + cy * size.cy;
    }
};

struct CPoint : public POINT
{
	CPoint(int x = 0, int y = 0)
	{
		this->x = x;
		this->y = y;
	}

	CPoint(const POINT& point)
	{
		this->x = point.x;
		this->y = point.y;
	}

	long GetDistance(POINT point) const
	{
		return (*this - point).Absolute();
	}

	CPoint operator +(const SIZE& size) const
	{
		return CPoint(x + size.cx, y + size.cy);
	}

	CSize operator -(const POINT& point) const
	{
		return CSize(x - point.x, y - point.y);
	}

    CPoint operator -(const SIZE& size) const
    {
        return CPoint(x - size.cx, y - size.cy);
    }

    static CPoint GetCenter(const CPoint& point1, const CPoint& point2)
    {
        return point1 + (point2 - point1) / 2;
    }
};

struct CLine
{
    CPoint start;
    CPoint end  ;

    CLine(CPoint start, CPoint end) : start(start), end(end)
    {}

    long GetDistance(CPoint point) const
    {
        auto d  = end   - start;
        auto d1 = start - point;
        auto d2 = end   - point;

        auto   f0 = d.GetInnerProduct(d1);
        auto   f1 = d.GetInnerProduct(d2);
        return f0 > 0 ? d1.Absolute()
                      : (f1 < 0 ? d2.Absolute() : ::labs(d.cy * d1.cx - d.cx * d1.cy) / d.Absolute());
    }
};

struct CRect : public RECT
{
    CPoint GetTopLeft() const
    {
        return CPoint(left, top);
    }

    CPoint GetTopRight() const
    {
        return CPoint(right, top);
    }

    CPoint GetBottomLeft() const
    {
        return CPoint(left, bottom);
    }

    CPoint GetBottomRight() const
    {
        return CPoint(right, bottom);
    }

    CPoint GetCenter() const
    {
        return CPoint(Math::Average(left, right), Math::Average(top, bottom));
    }

    CSize GetSize() const
    {
        return CSize(right - left, bottom - top);
    }

    CRect()
	{}
    
    CRect(const RECT& rect)
    {
        this->left   = rect.left  ;
        this->top    = rect.top   ;
        this->right  = rect.right ;
        this->bottom = rect.bottom;
    }

	CRect(POINT point1, POINT point2)
	{
		left   = Math::Min(point1.x, point2.x);
		top    = Math::Min(point1.y, point2.y);
		right  = Math::Max(point1.x, point2.x);
		bottom = Math::Max(point1.y, point2.y);
	}

    CRect(POINT point, SIZE size) : CRect(point, CPoint(point) + size)
    {}

    void Enlarge(POINT basePoint, double rate)
    {
        Enlarge(left  , basePoint.x, rate);
        Enlarge(top   , basePoint.y, rate);
        Enlarge(right , basePoint.x, rate);
        Enlarge(bottom, basePoint.y, rate);
    }

    CRect Intersect(const RECT& rect) const
    {
        CRect result;
        ::IntersectRect(&result, this, &rect);
        return result;
    }

    virtual long GetDistance(CPoint point) const
    {
        auto minimumDistance = LONG_MAX;
        if (point.x >= left && point.x <= right) {
            auto distance = GetDistance(point.y, top, bottom);
            if (distance < minimumDistance)
                minimumDistance = distance;
        }
        if (point.y >= top && point.y <= bottom) {
            auto distance = GetDistance(point.x, left, right);
            if (distance < minimumDistance)
                minimumDistance = distance;
        }
        for (int index = 0; index < 4; index++) {
            auto distance = point.GetDistance(GetCorner(index));
            if (distance < minimumDistance)
                minimumDistance = distance;
        }
        return minimumDistance;
    }

    vector<CPoint> GetCorners() const
    {
        vector<CPoint> corners;
        corners.push_back(GetTopLeft    ());
        corners.push_back(GetTopRight   ());
        corners.push_back(GetBottomRight());
        corners.push_back(GetBottomLeft ());
        return corners;
    }

    CRect GetInflateRect(long dx, long dy) const
    {
        CRect result(*this);
        ::InflateRect(&result, dx, dy);
        return result;
    }

private:
    static void Enlarge(long& value, long base, double rate)
    {
        value = Math::Round(base + (value - base) * rate);
    }

    static long GetDistance(long x, long value)
    {
        return labs(x - value);
    }

    static long GetDistance(long x, long value1, long value2)
    {
        return Math::Min(GetDistance(x, value1), GetDistance(x, value2));
    }

    CPoint GetCorner(int index) const
    {
        Debug::Assert(index >= 0 && index < 4);
        const CPoint corners[] = { GetTopLeft(),  GetTopRight(), GetBottomRight(), GetBottomLeft() };
        return corners[index];
    }
};

struct Circle
{
    CPoint center;
    long   radius;

    Circle() : radius(0)
    {}

    Circle(CPoint center, long radius) : center(center), radius(radius)
    {
        Debug::Assert(IsValid());
    }

    virtual bool IsValid() const
    {
        return radius > 0;
    }

    virtual CRect GetBoundRect() const
    {
        return CRect(CPoint(center.x - radius, center.y - radius),
                     CPoint(center.x + radius, center.y + radius));
    }

    long GetDistance(CPoint point) const
    {
        return ::labs(point.GetDistance(center) - radius);
    }
};

struct CEllipse : public CRect
{
    CEllipse()
    {}

    CEllipse(const RECT& rect) : CRect(rect)
    {}

    virtual long GetDistance(CPoint point) const
    {
        auto size = GetSize();
        if (size.cx == 0)
            return CLine(CPoint(left, top), CPoint(left, bottom)).GetDistance(point);
        if (size.cy == 0)
            return CLine(CPoint(left, top), CPoint(right, top  )).GetDistance(point);

        auto center = GetCenter();
        auto rate = static_cast<double>(size.cy) / size.cx;
        CPoint transformedPoint(center.x + Math::Round((point.x - center.x) * rate), point.y);

        return Math::Round(Circle(center, size.cy / 2).GetDistance(transformedPoint) / rate);
    }
};

class Color
{
public:
	static const COLORREF Black = RGB(0x00, 0x00, 0x00);
	static const COLORREF Red   = RGB(0xff, 0x00, 0x00);
	static const COLORREF Green = RGB(0x00, 0xff, 0x00);
	static const COLORREF Blue  = RGB(0x00, 0x00, 0xff);
};

class NullBrush
{
	HDC     hdc;
	HGDIOBJ oldBrush;

public:
	NullBrush(HDC hdc)
		: hdc(hdc), oldBrush(::SelectObject(hdc, ::GetStockObject(NULL_BRUSH)))
	{}

	virtual ~NullBrush()
	{
		::SelectObject(hdc, oldBrush);
	}
};

class SolidBrush
{
	HBRUSH hBrush;

public:
	HBRUSH GetHandle() const
	{
		return hBrush;
	}

	SolidBrush(COLORREF color)
		: hBrush(::CreateSolidBrush(color))
	{}

	virtual ~SolidBrush()
	{
		::DeleteObject(hBrush);
	}
};

class CDC : public Uncopyable
{
protected:
	HDC hdc;

public:
	HDC GetHandle() const
	{
		return this == nullptr ? nullptr : hdc;
	}

	virtual ~CDC() = 0;

	void MoveTo(POINT point) const
	{
		::MoveToEx(hdc, point.x, point.y, nullptr);
	}

	void LineTo(POINT point) const
	{
		::LineTo(hdc, point.x, point.y);
	}

    void Draw(const CLine& line) const
    {
        MoveTo(line.start);
        LineTo(line.end  );
    }

	void Rectangle(const RECT& rect) const
	{
		NullBrush nullBrush(hdc);
		::Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	}

	void Ellipse(const RECT& rect) const
	{
		NullBrush nullBrush(hdc);
		::Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
	}

	void FillRect(const RECT& area, COLORREF color) const
	{
		SolidBrush brush(color);
		::FillRect(hdc, &area, brush.GetHandle());
	}

    int DrawText(tstring text, RECT& area, UINT format) const
    {
        auto buffer = unique_ptr<TCHAR>(new TCHAR[text.size() + 4]);
        lstrcpy(buffer.get(), text.c_str());
        return ::DrawText(hdc, buffer.get(), -1, &area, format);
    }

    COLORREF SetTextColor(COLORREF color) const
    {
        return ::SetTextColor(hdc, color);
    }

    int SetBkMode(int backMode) const
    {
        return ::SetBkMode(hdc, backMode);
    }

	int SetROP2(int drawMode) const
	{
		return ::SetROP2(hdc, drawMode);
	}

    int SetMapMode(int mode) const
    {
        return ::SetMapMode(hdc, mode);
    }

    bool SetWindowOrg(POINT logicalPoint) const
    {
        return ::SetWindowOrgEx(hdc, logicalPoint.x, logicalPoint.y, nullptr);
    }

    bool SetWindowExt(SIZE logicalSize) const
    {
        return ::SetWindowExtEx(hdc, logicalSize.cx, logicalSize.cy, nullptr);
    }

    bool SetViewportOrg(POINT phisicalPoint) const
    {
        return ::SetViewportOrgEx(hdc, phisicalPoint.x, phisicalPoint.y, nullptr);
    }

    bool SetViewportExt(SIZE phisicalSize) const
    {
        return ::SetViewportExtEx(hdc, phisicalSize.cx, phisicalSize.cy, nullptr);
    }

    bool DPtoLP(POINT& point) const
    {
        return ::DPtoLP(hdc, &point, 1);
    }

    bool LPtoDP(POINT& point) const
    {
        return ::LPtoDP(hdc, &point, 1);
    }

    void LPtoDP(RECT& rect) const
    {
        CRect target(rect);
        auto  topLeft     = target.GetTopLeft    ();
        LPtoDP(topLeft    );
        auto  bottomRight = target.GetBottomRight();
        LPtoDP(bottomRight);
        CRect result(topLeft, bottomRight);
        rect = result;
    }

    void DPtoLP(long& distance) const
    {
        CPoint point1(0, 0);
        auto d = Math::Round(sqrt(Math::Square(distance) / 2));
        CPoint point2(d, d);

        DPtoLP(point1);
        DPtoLP(point2);
        distance = Math::Round(point1.GetDistance(point2));
    }

    void LPtoDP(long& distance) const
    {
        CPoint point1(0, 0);
        auto d = Math::Round(sqrt(Math::Square(distance) / 2));
        CPoint point2(d, d);

        LPtoDP(point1);
        LPtoDP(point2);
        distance = Math::Round(point1.GetDistance(point2));
    }
};

inline CDC::~CDC() {}

class CWnd;
class CPaintDC : public CDC
{
	CWnd&       window;
	PAINTSTRUCT ps;

public:
	CPaintDC(CWnd& window);
	virtual ~CPaintDC();
};

class CClientDC : public CDC
{
	CWnd& window;

public:
	CClientDC(CWnd& window);
	virtual ~CClientDC();
};

class CAttachedDC : public CDC
{
public:
	CAttachedDC(HDC hdc)
	{
		this->hdc = hdc;
	}
};

template <class THandle>
class CGdiObj : public Uncopyable
{
protected:
    const THandle hObject;
    HGDIOBJ       hOldObject;

    CGdiObj(THandle hObject) : hObject(hObject)
    {
        if (hObject == nullptr)
            throw exception();
    }

public:
    THandle GetHandle() const
    {
        return this == nullptr ? nullptr : hObject;
    }

    virtual ~CGdiObj()
    {
        if (hObject != nullptr)
            ::DeleteObject(hObject);
    }

    void AttachTo(CDC& dc)
    {
        hOldObject = ::SelectObject(dc.GetHandle(), GetHandle());
    }

    void DetachFrom(CDC& dc)
    {
        ::SelectObject(dc.GetHandle(), hOldObject);
    }
};

class CPen : public CGdiObj<HPEN>
{
public:
	CPen(int style = PS_SOLID, int width = 0, COLORREF color = Color::Black)
		: CGdiObj<HPEN>(::CreatePen(style, width, color))
	{}
};

class CFont : public CGdiObj<HFONT>
{
public:
    CFont(const LOGFONT& logFont)
        : CGdiObj<HFONT>(::CreateFontIndirect(&logFont))
    {}
};

template <class TCGdiObj>
class CGdiObjSelector : public Uncopyable
{
	CDC&                 dc;
    unique_ptr<TCGdiObj> obj;
	bool                 isMine;

public:
    CGdiObjSelector(CDC& dc, TCGdiObj& obj, bool isMine) : dc(dc), obj(&obj), isMine(isMine)
	{
		obj.AttachTo(dc);
	}

	virtual ~CGdiObjSelector()
	{
        obj->DetachFrom(dc);
        if (!isMine)
            obj.release();
	}
};

class PenSelector : public CGdiObjSelector<CPen>
{
public:
    PenSelector(CDC& dc, CPen& pen)
        : CGdiObjSelector<CPen>(dc, pen, false)
    {}

    PenSelector(CDC& dc, int style = PS_SOLID, int width = 0, COLORREF color = Color::Black)
        : CGdiObjSelector<CPen>(dc, *new CPen(style, width, color), true)
    {}
};

class FontSelector : public CGdiObjSelector<CFont>
{
public:
    FontSelector(CDC& dc, CFont& font)
        : CGdiObjSelector<CFont>(dc, font, false)
    {}

    FontSelector(CDC& dc, const LOGFONT& logFont)
        : CGdiObjSelector<CFont>(dc, *new CFont(logFont), true)
    {}
};

class CWnd
{
	static const _TCHAR windowClassName[];            // メイン ウィンドウ クラス名

protected:
	HINSTANCE hInstance;
	HWND      hWnd;
    bool      isAttached;

public:
	HWND GetSafeHwnd() const
	{
		return this == nullptr ? nullptr : hWnd;
	}

	CWnd(HINSTANCE hInstance = nullptr) : hInstance(hInstance), hWnd(nullptr), isAttached(false)
	{}

    virtual ~CWnd()
    {
        Destroy();
    }

    bool Destroy()
    {
        if (isAttached || hWnd == nullptr)
            return false;
        ::DestroyWindow(hWnd);
        hWnd = nullptr;
        return true;
    }

	bool Create(CWnd* parent = nullptr, tstring title = _T(""), UINT style = WS_OVERLAPPEDWINDOW, UINT menuId = 0, int nCmdShow = SW_SHOW)
	{
		RegisterWindowClass(menuId);
		return Create(parent, title, style, nCmdShow);
	}

    CWnd& Attach(HWND hWnd)
    {
        Destroy();
        isAttached      = true;
        this->hWnd      = hWnd;
        this->hInstance = HINSTANCE(::GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
        return *this;
    }

    HWND Detach()
    {
        isAttached = false;
        auto hWnd  = this->hWnd;
        this->hWnd = nullptr;
        return hWnd;
    }

	bool Move(const RECT& area) const
	{
		return ::MoveWindow(GetSafeHwnd(), area.left, area.top, area.right - area.left, area.bottom - area.top, TRUE);
	}

    bool Show(int showCommand = SW_SHOW)
    {
        return ::ShowWindow(GetSafeHwnd(), showCommand);
    }

    void SetFocus()
    {
        ::SetFocus(GetSafeHwnd());
    }

    void Set(const CFont& font)
    {
        SetWindowFont(GetSafeHwnd(), font.GetHandle(), true);
    }

    bool SetText(tstring text) const
    {
        return ::SetWindowText(GetSafeHwnd(), text.c_str());
    }

    tstring GetText() const
    {
        const int bufferSize = 1024;
        TCHAR     buffer[bufferSize];
        ::GetWindowText(GetSafeHwnd(), buffer, bufferSize - 2);
        buffer[bufferSize - 1] = TCHAR('\0');
        return tstring(buffer);
    }

	CRect GetClientArea() const
	{
		CRect clientArea;
		::GetClientRect(GetSafeHwnd(), &clientArea);
		return clientArea;
	}

	void Invalidate(const RECT* area = nullptr, bool bErase = true)
	{
		::InvalidateRect(GetSafeHwnd(), area, bErase);
	}

    bool ShowScrollBar(int bar, bool show = true)
    {
        return ::ShowScrollBar(hWnd, bar, show);
    }

    bool SetScrollInfo(int bar, const SCROLLINFO* scrollInfo, bool redraw = true)
    {
        return ::SetScrollInfo(hWnd, bar, scrollInfo, redraw);
    }

    bool GetScrollInfo(int bar, SCROLLINFO* scrollInfo)
    {
        return ::GetScrollInfo(hWnd, bar, scrollInfo);
    }

protected:
	virtual LRESULT OnCommand(UINT notificationCode, int commandId)
	{
		return 0;
	}

    virtual void OnPrepareDC(CDC& dc)
    {}

	virtual void OnDraw(CDC& dc)
	{}

	virtual void OnEraseBackground(CDC& dc)
	{}

	virtual void OnLButtonDown(UINT keys, POINT point)
	{}

	virtual void OnLButtonUp(UINT keys, POINT point)
	{}

	virtual void OnMouseMove(UINT keys, POINT point)
	{}

    virtual void OnMouseLeave()
    {}

    virtual void OnMouseWheel(UINT keys, double delta, POINT point)
    {}

    virtual void OnHScroll(UINT code, UINT position, CWnd* pScrollBar)
    {}

    virtual void OnVScroll(UINT code, UINT position, CWnd* pScrollBar)
    {}

    virtual void OnCreate()
	{}

	virtual void OnDestroy()
	{}

	virtual void OnSize()
	{}

private:
	ATOM RegisterWindowClass(UINT menuId = 0)
	{
		WNDCLASSEXW wcex;
		wcex.cbSize        = sizeof(WNDCLASSEX);
		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc   = WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = 0;
		wcex.hInstance     = hInstance;
		wcex.hIcon         = nullptr;
		wcex.hCursor       = ::LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
		wcex.lpszMenuName  = menuId == 0 ? nullptr : MAKEINTRESOURCEW(menuId);
		wcex.lpszClassName = windowClassName;
		wcex.hIconSm       = nullptr;

		return RegisterClassEx(&wcex);
	}

	bool Create(CWnd* parent, tstring title, UINT style, int nCmdShow)
	{
		hWnd = ::CreateWindow(windowClassName, title.c_str(), style,
							  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, parent->GetSafeHwnd(), nullptr, hInstance, this);
		if (hWnd == nullptr)
			return false;

		::ShowWindow(hWnd, nCmdShow);
		::UpdateWindow(hWnd);
		return true;
	}

	LRESULT WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_COMMAND:
		{
            UINT notificationCode = HIWORD(wParam);
            int  commandId        = LOWORD(wParam);
			return OnCommand(notificationCode, commandId);
		}
		break;
		case WM_PAINT:
		{
			CPaintDC dc(*this);
            OnPrepareDC(dc);
			OnDraw(dc);
		}
		break;
		case WM_SIZE:
			OnSize();
			break;
		case WM_ERASEBKGND:
		{
			auto hdc = HDC(wParam);
			CAttachedDC dc(hdc);
			OnEraseBackground(dc);
		}
		break;
		case WM_LBUTTONDOWN:
		{
			const POINT point = { GET_X_LPARAM(lParam),
								  GET_Y_LPARAM(lParam) };
			OnLButtonDown(UINT(wParam), point);
		}
		break;
		case WM_LBUTTONUP:
		{
			const POINT point = { GET_X_LPARAM(lParam),
								  GET_Y_LPARAM(lParam) };
			OnLButtonUp(UINT(wParam), point);
		}
		break;
		case WM_MOUSEMOVE:
		{
			const POINT point = { GET_X_LPARAM(lParam),
								  GET_Y_LPARAM(lParam) };
			OnMouseMove(UINT(wParam), point);
            TrackMouseEvent();
		}
		break;
        case WM_MOUSELEAVE:
            OnMouseLeave();
            break;

        case WM_MOUSEWHEEL:
        {
            const auto  keys  = UINT(LOWORD(wParam));
            const auto  delta = short(HIWORD(wParam)) / (double)WHEEL_DELTA;
            const POINT point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            OnMouseWheel(keys, delta, point);
        }
        break;
        case WM_HSCROLL :
        {
            UINT  code     = LOWORD(wParam);
            UINT  position = HIWORD(wParam);
            static CWnd temporaryScrollBar;
            CWnd* scrollBar = HWND(lParam) == nullptr ? nullptr : &temporaryScrollBar.Attach(HWND(lParam));
            OnHScroll(code, position, scrollBar);
        }
        break;
        case WM_VSCROLL:
        {
            UINT  code     = LOWORD(wParam);
            UINT  position = HIWORD(wParam);
            static CWnd temporaryScrollBar;
            CWnd* scrollBar = HWND(lParam) == nullptr ? nullptr : &temporaryScrollBar.Attach(HWND(lParam));
            OnVScroll(code, position, scrollBar);
        }
        break;
        case WM_DESTROY:
			OnDestroy();
			break;
		default:
			return ::DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}

    void TrackMouseEvent()
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize    = sizeof(tme);
        tme.dwFlags   = TME_LEAVE;
        tme.hwndTrack = GetSafeHwnd();
        ::TrackMouseEvent(&tme);
    }

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
        switch (message)
        {
        case WM_CREATE:
        {
            auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            auto self         = static_cast<CWnd*>(createStruct->lpCreateParams);
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hWnd = hWnd;
            self->OnCreate();
            return 0;
        }
        break;
        default:
        {
            auto self = reinterpret_cast<CWnd*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            return self == nullptr ? ::DefWindowProc(hWnd, message, wParam, lParam)
                                   : self->WindowProcedure(hWnd, message, wParam, lParam);
        }
        break;
        }
	}
};

const _TCHAR CWnd::windowClassName[] = _T("FCWindowClass");

// CPaintDC

inline CPaintDC::CPaintDC(CWnd& window) : window(window)
{
	hdc = ::BeginPaint(window.GetSafeHwnd(), &ps);
}

inline CPaintDC::~CPaintDC()
{
	::EndPaint(window.GetSafeHwnd(), &ps);
}

// CClientDC

inline CClientDC::CClientDC(CWnd& window) : window(window)
{
	hdc = ::GetDC(window.GetSafeHwnd());
}

inline CClientDC::~CClientDC()
{
	::ReleaseDC(window.GetSafeHwnd(), hdc);
}

class CEdit : public CWnd
{
public:
    CEdit(HINSTANCE hInstance = nullptr) : CWnd(hInstance)
    {}

    bool Create(CWnd* parent, tstring text, UINT style, const RECT& area, UINT id = 0)
    {
        hWnd = ::CreateWindow(_T("EDIT"), text.c_str(), style,
                              area.left, area.top, area.right - area.left, area.bottom - area.top,
                              parent->GetSafeHwnd(), HMENU(id), hInstance, nullptr);
        return hWnd != nullptr;
    }
};

} // namespace Windows

namespace CadCore {
using namespace Windows;

const long modelSize = 1000000;

class Figure
{
    const long defaultSelectorWidth = 10;
    const COLORREF selectorColor    = RGB(0x80, 0x80, 0x80);

    bool     isSelected;
	COLORREF color;

public:
    virtual unique_ptr<Figure> Clone() const = 0;

    bool IsSelected() const
    {
        return isSelected;
    }

    void Select(bool isSelected = true)
    {
        this->isSelected = isSelected;
    }

    void ToggleSelect()
    {
        Select(!IsSelected());
    }

	COLORREF GetColor() const
	{
		return color;
	}

	void SetColor(COLORREF color)
	{
		this->color = color;
	}

    virtual long GetDistance(CPoint point)
    {
        return LONG_MAX;
    }   

    CRect GetDrawingBoundRect(CDC& dc)
    {
        auto selectorWidth = defaultSelectorWidth;
        dc.DPtoLP(selectorWidth);
        const auto d = selectorWidth / 2 + 1;
        return GetBoundRect().GetInflateRect(d, d);
    }

    virtual CRect GetBoundRect()
    {
        CPoint minimum(LONG_MAX, LONG_MAX);
        CPoint maximum(LONG_MIN, LONG_MIN);
        for (auto point : GetPoints()) {
            minimum.x = Math::Min(minimum.x, point.x);
            minimum.y = Math::Min(minimum.y, point.y);
            maximum.x = Math::Max(maximum.x, point.x);
            maximum.y = Math::Max(maximum.y, point.y);
        }
        return CRect(minimum, maximum);
    }

    virtual vector<CPoint> GetPoints()
    {
        return vector<CPoint>();
    }

	Figure() : isSelected(false), color(Color::Black)
	{}

    void Draw(CDC& dc)
    {
        PenSelector penSelector(dc, PS_SOLID, 0, GetColor());
        DrawShape(dc);
        DrawSelectors(dc);
    }

protected:
    virtual void DrawShape(CDC& dc)
    {}

private:
    void DrawSelectors(CDC& dc)
    {
        PenSelector penSelector(dc, PS_SOLID, 0, selectorColor);
        for (auto point : GetPoints())
            DrawSelector(dc, point);
    }

    void DrawSelector(CDC& dc, CPoint point)
    {
        if (!IsSelected())
            return;

        auto selectorWidth = defaultSelectorWidth;
        dc.DPtoLP(selectorWidth);

        CSize selectorSize(selectorWidth, selectorWidth);
        CRect rect(point - selectorSize / 2, selectorSize);
       
        dc.Rectangle(rect);
    }
};

struct UndoData
{
public:
    enum Operation {
        None, Add, Delete, Update
    };

    Operation          operation;
    shared_ptr<Figure> oldFigure;
    shared_ptr<Figure> newFigure;

    static UndoData AddData(shared_ptr<Figure> newFigure)
    {
        return UndoData(Add, nullptr, newFigure);
    }

    static UndoData DeleteData(shared_ptr<Figure> oldFigure)
    {
        return UndoData(Delete, oldFigure, nullptr);
    }

    static UndoData UpdateData(shared_ptr<Figure> oldFigure, shared_ptr<Figure> newFigure)
    {
        return UndoData(Update, oldFigure, newFigure);
    }

    UndoData Invert() const
    {
        return UndoData(Invert(operation), newFigure, oldFigure);
    }

private:
    UndoData(Operation operation, shared_ptr<Figure> oldFigure, shared_ptr<Figure> newFigure)
        : operation(operation), oldFigure(oldFigure), newFigure(newFigure)
    {}

    static Operation Invert(Operation operation)
    {
        switch (operation) {
            case Add   : return Delete;
            case Delete: return Add   ;
            case Update: return Update;
            default    : Debug::Assert(false);
        }
        return None;
    }
};

class UndoDataGroup
{
    vector<UndoData> undoDataList;

public:
    typedef vector<UndoData>::iterator iterator;

    bool IsEmpty() const
    {
        return undoDataList.size() == 0;
    }

    void Add(const UndoData& undoData)
    {
        undoDataList.push_back(undoData);
    }

    iterator begin()
    {
        return undoDataList.begin();
    }

    iterator end()
    {
        return undoDataList.end();
    }

    size_t size() const
    {
        return undoDataList.size();
    }

    const UndoData& operator[](size_t index) const
    {
        return undoDataList[index];
    }
};

class UndoBuffer : public Uncopyable
{
    shared_ptr<UndoDataGroup>         currentUndoDataGroup;
    vector<shared_ptr<UndoDataGroup>> undoList;
    size_t                            currentIndex;

public:
    bool CanUndo() const
    {
        return currentIndex > 0;
    }

    bool CanRedo() const
    {
        return currentIndex < undoList.size();
    }

    UndoBuffer() : currentUndoDataGroup(nullptr), currentIndex(0)
    {}

    void Start()
    {
        Flush();
        currentUndoDataGroup.reset(new UndoDataGroup());
    }

    void End()
    {
        Flush();
    }

    void PushAddData(shared_ptr<Figure> newFigure)
    {
        Push(UndoData::AddData(newFigure));
    }

    void PushDeleteData(shared_ptr<Figure> oldFigure)
    {
        Push(UndoData::DeleteData(oldFigure));
    }

    void PushUpdateData(shared_ptr<Figure> oldFigure, shared_ptr<Figure> newFigure)
    {
        Push(UndoData::UpdateData(oldFigure, newFigure));
    }

    bool Undo(vector<UndoData>& undoDataList)
    {
        if (CanUndo()) {
            UndoDataGroup& undoDataGroup = *undoList[--currentIndex];
            for (long index = undoDataGroup.size() - 1; index >= 0; index--)
                undoDataList.push_back(undoDataGroup[index].Invert());
            return true;
        }
        return false;
    }

    bool Redo(vector<UndoData>& undoDataList)
    {
        if (CanRedo()) {
            UndoDataGroup& undoDataGroup = *undoList[--currentIndex];
            for (auto index = 0U; index < undoDataGroup.size(); index++)
                undoDataList.push_back(undoDataGroup[index]);
            return true;
        }
        return false;
    }

private:
    void Push(const UndoData& undoData)
    {
        if (currentUndoDataGroup == nullptr)
            Start();
        currentUndoDataGroup->Add(undoData);
    }

    void Flush()
    {
        if (currentUndoDataGroup != nullptr && !currentUndoDataGroup->IsEmpty()) {
            undoList.push_back(currentUndoDataGroup);
            currentIndex++;
        } else {
            currentUndoDataGroup.reset();
        }
    }
};

class UndoScope
{
    UndoBuffer& undoBuffer;

public:
    UndoScope(UndoBuffer& undoBuffer) : undoBuffer(undoBuffer)
    {
        undoBuffer.Start();
    }

    virtual ~UndoScope()
    {
        undoBuffer.End();
    }

    void PushAddData(shared_ptr<Figure> newFigure) const
    {
        undoBuffer.PushAddData(newFigure);
    }

    void PushDeleteData(shared_ptr<Figure> oldFigure) const
    {
        undoBuffer.PushDeleteData(oldFigure);
    }

    void PushUpdateData(shared_ptr<Figure> oldFigure, shared_ptr<Figure> newFigure) const
    {
        undoBuffer.PushUpdateData(oldFigure, newFigure);
    }
};

class CadData : public Observable, public Uncopyable
{
    CRect                      area;
	vector<shared_ptr<Figure>> figures;
	COLORREF                   currentColor;
    UndoBuffer                 undoBuffer;

public:
	typedef vector<shared_ptr<Figure>>::const_iterator iterator;

    const CRect& GetArea() const
    {
        return area;
    }

	COLORREF GetCurrentColor() const
	{
		return currentColor;
	}

	void SetCurrentColor(COLORREF color)
	{
		currentColor = color;
	}

	CadData() : area(CPoint(), CSize(modelSize, modelSize)), currentColor(Color::Black)
	{}

	iterator begin() const
	{
		return figures.begin();
	}

	iterator end() const
	{
		return figures.end();
	}

	void Add(unique_ptr<Figure> figure)
	{
        const UndoScope undoScope(undoBuffer);
        
        SelectAll(false);
        figure->Select();
        figure->SetColor(GetCurrentColor());
        auto newFigure = shared_ptr<Figure>(figure.release());
        figures.push_back(newFigure);
        undoScope.PushAddData(newFigure);
        Update(figure.get());
    }

    void Delete(bool update = true)
    {
        const UndoScope undoScope(undoBuffer);

        figures.erase(
            remove_if(
                figures.begin(), figures.end(),
                [&](shared_ptr<Figure> figure) {
                    if (figure->IsSelected()) {
                        undoScope.PushDeleteData(figure);
                        return true;
                    }
                    return false;
                }
            ),
            figures.end()
        );
        if (update)
            Update(nullptr);
    }

    void ToggleSelect(POINT point, long minimumDistance)
    {
        auto targetFigure = Search(point, minimumDistance);
        if (targetFigure != nullptr) {
            targetFigure->ToggleSelect();
            Update(targetFigure.get());
        }
    }

    void SelectAlone(POINT point, long minimumDistance)
    {
        SelectAll(false);
        auto targetFigure = Search(point, minimumDistance);
        if (targetFigure != nullptr) {
            targetFigure->Select();
            Update(targetFigure.get());
        }
    }

    void Undo()
    {
        vector<UndoData> undoDataList;
        if (undoBuffer.Undo(undoDataList))
            Do(undoDataList);
    }
    
    void Redo()
    {
        vector<UndoData> undoDataList;
        if (undoBuffer.Redo(undoDataList))
            Do(undoDataList);
    }

private:
    shared_ptr<Figure> Search(POINT point, long minimumDistance)
    {
        shared_ptr<Figure> targetFigure = nullptr;
        for (auto figure : figures) {
            auto distance = figure->GetDistance(point);
            if (distance < minimumDistance) {
                minimumDistance = distance;
                targetFigure    = figure;
            }
        }
        return targetFigure;
    }

    void SelectAll(bool isSelected = true, bool update = true)
    {
        for_each(figures.begin(), figures.end(), [&](shared_ptr<Figure> figure) {
            if (figure->IsSelected() != isSelected) {
                figure->Select(isSelected);
                if (update)
                    Update(figure.get());
            }
        });
    }

    void Do(const vector<UndoData>& undoDataList)
    {
        for (auto index = 0U; index < undoDataList.size(); index++)
            Do(undoDataList[index]);
    }

    void Do(const UndoData& undoData)
    {
        switch (undoData.operation) {
            case UndoData::Add:
                figures.push_back(undoData.newFigure);
                Update(undoData.newFigure.get());
                break;
            case UndoData::Delete:
                for (auto index = 0u; index < figures.size(); index++) {
                    if (figures[index].get() == undoData.oldFigure.get()) {
                        Update(undoData.oldFigure.get());
                        figures.erase(figures.begin() + index);
                        break;
                    }
                }
                //figures.erase(remove(figures.begin(), figures.end(), undoData.oldFigure.get()), figures.end());
                break;
            case UndoData::Update:
                break;
        }
    }
};

class RubberBandHolder
{
public:
	virtual void DrawFigure(CDC& dc, POINT point) = 0;
};

class RubberBand
{
	POINT point;
	bool  hasPoint;

	RubberBandHolder& holder;

public:
	void Set(CDC& dc, POINT point)
	{
		if (hasPoint)
			Erase(dc);
		else
			hasPoint = true;
		this->point = point;
		Draw(dc);
	}

	RubberBand(RubberBandHolder& holder)
		: hasPoint(false), holder(holder)
	{}

	void Draw(CDC& dc)
	{
		DrawFigure(dc);
	}

	void Reset(CDC& dc)
	{
		Erase(dc);
		hasPoint = false;
	}

private:
	void Erase(CDC& dc)
	{
		DrawFigure(dc);
	}

	void DrawFigure(CDC& dc)
	{
		auto oldDrawMode = dc.SetROP2(R2_NOT);
        holder.DrawFigure(dc, point);
		dc.SetROP2(oldDrawMode);
	}
};

class CadView;
class Command
{
protected:
	CadData& cadData;
    CadView& cadView;

public:
    Command(CadData& cadData, CadView& cadView) : cadData(cadData), cadView(cadView)
    {}

    virtual void OnClick(CDC& dc, UINT keys, POINT point)
    {}

	virtual void OnDragStart(CDC& dc, POINT point)
	{}

    virtual void OnDragging(CDC& dc, POINT point)
    {}

    virtual void OnDragEnd(CDC& dc, POINT point)
    {}

    virtual void OnDragStop(CDC& dc)
    {}

	virtual void OnDrawRubberBand(CDC& dc, POINT point)
	{}
};

class SelectCommand : public Command
{
    static const long selectingMinimumDistance = 10;

public:
    SelectCommand(CadData& cadData, CadView& cadView) : Command(cadData, cadView)
    {}

    virtual void OnClick(CDC& dc, UINT keys, POINT point)
    {
        auto logicalSelectingMinimumDistance = selectingMinimumDistance;
        dc.DPtoLP(logicalSelectingMinimumDistance);

        if ((keys & MK_CONTROL) == 0)
            cadData.SelectAlone (point, logicalSelectingMinimumDistance);
        else
            cadData.ToggleSelect(point, logicalSelectingMinimumDistance);
    }
};

class AddCommand : public Command
{
protected:
	POINT        firstPoint;

public:
	AddCommand(CadData& cadData, CadView& cadView) : Command(cadData, cadView)
	{}

	virtual void OnDragStart(CDC& dc, POINT point)
	{
		firstPoint = point;
	}

	virtual void OnDragEnd(CDC& dc, POINT point)
	{
		cadData.Add(CreateFigure(point));
	}

	virtual void OnDrawRubberBand(CDC& dc, POINT point)
	{
		auto figure = CreateFigure(point);
		if (figure != nullptr)
			figure->Draw(dc);
	}

protected:
	virtual unique_ptr<Figure> CreateFigure(POINT point) const = 0;
};

class CommandManager : public RubberBandHolder, public Uncopyable
{
	CadData&            cadData;
	RubberBand          rubberBand;
	unique_ptr<Command> command;

public:
	void SetCommand(unique_ptr<Command> command)
	{
		this->command.reset(command.release());
	}

	CommandManager(CadData& cadData, CadView& cadView)
		: cadData(cadData), rubberBand(*this)
	{
		command = unique_ptr<Command>(new SelectCommand(cadData, cadView));
	}

	void OnDraw(CDC& dc)
	{
		rubberBand.Draw(dc);
		for (auto figure : cadData)
			figure->Draw(dc);
	}

    void OnClick(CDC& dc, UINT keys, POINT point)
    {
        command->OnClick(dc, keys, point);
    }

	void OnDragStart(CDC& dc, POINT point)
	{
		command->OnDragStart(dc, point);
	}

    void OnDragging(CDC& dc, POINT point)
    {
        command->OnDragging(dc, point);
        rubberBand.Set(dc, point);
    }

    void OnDragStop(CDC& dc)
    {
        rubberBand.Reset(dc);
        command->OnDragStop(dc);
    }

	void OnDragEnd(CDC& dc, POINT point)
	{
		rubberBand.Reset(dc);
		command->OnDragEnd(dc, point);
	}

private:
	virtual void DrawFigure(CDC& dc, POINT point)
	{
		command->OnDrawRubberBand(dc, point);
	}
};

class MouseEventConverter
{
    static const long  dragStartDistance = 10;
    bool               isDown;
    bool               isDragging;
    vector<POINT> mouseMovePositions;

public:
    MouseEventConverter() : isDown(false), isDragging(false)
    {}

    void OnLButtonDown(UINT keys, POINT point)
    {
        Reset();
        isDown = true;
        mouseMovePositions.push_back(point);
    }

    void OnMouseMove(UINT keys, POINT point)
    {
        if (isDown && (keys & MK_LBUTTON) != 0) {
            if (isDragging) {
                OnDragging(point);
            } else {
                Debug::Assert(mouseMovePositions.size() > 0);
                if (CPoint(point).GetDistance(mouseMovePositions[0]) >= dragStartDistance) {
                    OnDragStart(mouseMovePositions[0]);
                    for (auto mouseMovePosition : mouseMovePositions)
                        OnDragging(mouseMovePosition);
                    mouseMovePositions.clear();
                    isDragging = true;
                } else {
                    mouseMovePositions.push_back(point);
                }
            }
        }
    }

    void OnLButtonUp(UINT keys, POINT point)
    {
        if (isDown) {
            if (isDragging)
                OnDragEnd(point);
            else
                OnClick(keys, point);
        }
        Reset();
    }

    void OnMouseLeave()
    {
        if (isDown && isDragging) {
            Reset();
            OnDragStop();
        }
    }

protected:
    virtual void OnClick(UINT keys, POINT point)
    {}

    virtual void OnDragStart(POINT point)
    {}

    virtual void OnDragging(POINT point)
    {}

    virtual void OnDragEnd(POINT point)
    {}

    virtual void OnDragStop()
    {}

private:
    void Reset()
    {
        isDown = isDragging = false;
        mouseMovePositions.clear();
    }
};

class StandardLogFont
{
    LOGFONT logFont;

public:
    static const long defaultHeight = modelSize / 30;

    StandardLogFont(long height = defaultHeight)
    {
        ::ZeroMemory(&logFont, sizeof(LOGFONT));
        logFont.lfHeight         = -height;
        logFont.lfWeight         = FW_NORMAL;
        logFont.lfCharSet        = DEFAULT_CHARSET;
        logFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
        logFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        logFont.lfQuality        = PROOF_QUALITY;
        logFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        lstrcpy(logFont.lfFaceName, _T("Meiryo"));
    }

    operator const LOGFONT&()
    {
        return logFont;
    }
};


const UINT WM_EDITCANCEL = WM_USER + 100;

class Editor : public CEdit
{
    unique_ptr<CFont> font;
    long              logicalFontHeight;
    RECT              logicalArea;

public:
    void Set(CDC& dc, tstring text, long logicalFontHeight, const RECT& logicalArea)
    {
        this->logicalFontHeight = logicalFontHeight;
        this->logicalArea       = logicalArea      ;
        Set(dc, text);
    }

    void Reset(CDC& dc)
    {
        Set(dc, GetText());
    }

private:
    void Set(CDC& dc, tstring text)
    {
        auto  fontHeight = logicalFontHeight;
        dc.LPtoDP(fontHeight);
        CRect area       = logicalArea;
        dc.LPtoDP(area);

        Set(text, fontHeight, area);
    }

    void Set(tstring text, long fontHeight, const RECT& area)
    {
        StandardLogFont logFont(fontHeight);
        font.reset(new CFont(logFont));

        SetText(text);
        CEdit::Set(*font);
        Move(area);
        SetFocus();
        Show();
    }
};

class CadView : public CWnd, public MouseEventConverter, public Observer
{
    static const COLORREF backgroundColor = RGB(0xff, 0xff, 0xc0);
    static const COLORREF paperColor      = RGB(0xff, 0xff, 0xff);
    static const UINT     editId          = 100;

    CommandManager&   commandManager;
	CadData&          cadData;
    CRect             logicalArea;

    Editor            editor;

public:
	CadView(HINSTANCE hInstance, CadData& cadData, CommandManager& commandManager)
		: CWnd(hInstance), cadData(cadData), commandManager(commandManager), logicalArea(cadData.GetArea())
	{
        cadData.AddObserver(*this);
    }

    bool Create(CWnd* parent)
	{
		return CWnd::Create(parent, _T(""), WS_CHILDWINDOW);
	}

    void Home()
    {
        SetLogicalArea(cadData.GetArea());
    }

    void SetEdit(tstring text, long fontHeight, const RECT& area)
    {
        CClientDC dc(*this);
        OnPrepareDC(dc);
        editor.Set(dc, text, fontHeight, area);
    }

protected:
    virtual void OnCreate()
    {
        editor.Create(this, _T("     "),
                      WS_CHILD | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | ES_WANTRETURN,
                      CRect(CPoint(100, 100), CSize(200, 40)), editId);
    }

    virtual void OnPrepareDC(CDC& dc)
    {
        dc.SetMapMode(MM_ISOTROPIC);
        dc.SetWindowOrg  (logicalArea.GetCenter());
        dc.SetWindowExt  (logicalArea.GetSize  ());

        auto clientArea = GetClientArea();
        dc.SetViewportOrg(clientArea.GetCenter());
        dc.SetViewportExt(clientArea.GetSize  ());
    }
    
    virtual void OnDraw(CDC& dc)
	{
        DrawPaper(dc);
        commandManager.OnDraw(dc);
		for (auto figure : cadData)
			figure->Draw(dc);
	}

	virtual void OnEraseBackground(CDC& dc)
	{
		dc.FillRect(GetClientArea(), backgroundColor);
	}

	virtual void OnLButtonDown(UINT keys, POINT point)
	{
        MouseEventConverter::OnLButtonDown(keys, point);
	}

	virtual void OnLButtonUp(UINT keys, POINT point)
	{
        MouseEventConverter::OnLButtonUp(keys, point);
	}

	virtual void OnMouseMove(UINT keys, POINT point)
	{
        MouseEventConverter::OnMouseMove(keys, point);
	}

    virtual void OnMouseLeave()
    {
        MouseEventConverter::OnMouseLeave();
    }

    virtual void OnMouseWheel(UINT keys, double delta, POINT point)
    {
        if ((keys & MK_CONTROL) != 0) {
            DPtoLP(point);

            const auto denominator = 10.0;
            auto newLogicalArea = logicalArea;
            Enlarge(newLogicalArea, point, (denominator - delta) / denominator);
            SetLogicalArea(newLogicalArea);
        }
    }

    virtual void OnHScroll(UINT code, UINT position, CWnd* pScrollBar)
    {
        CRect newLogicalArea;
        switch (code) {
        case SB_LEFT:
            newLogicalArea = CRect(CPoint(cadData.GetArea().left, logicalArea.top), logicalArea.GetSize());
            break;
        case SB_RIGHT:
            newLogicalArea = CRect(CPoint(cadData.GetArea().right - logicalArea.GetSize().cx, logicalArea.top), logicalArea.GetSize());
            break;
        case SB_LINELEFT:
            newLogicalArea = CRect(CPoint(Math::Max(logicalArea.left - logicalArea.GetSize().cx / 10,  cadData.GetArea().left), logicalArea.top), logicalArea.GetSize());
            break;
        case SB_LINERIGHT:
            newLogicalArea = CRect(CPoint(Math::Min(logicalArea.left + logicalArea.GetSize().cx / 10, cadData.GetArea().right - logicalArea.GetSize().cx), logicalArea.top), logicalArea.GetSize());
            break;
        case SB_PAGELEFT:
            newLogicalArea = CRect(CPoint(Math::Max(logicalArea.left - logicalArea.GetSize().cx / 2, cadData.GetArea().left), logicalArea.top), logicalArea.GetSize());
            break;
        case SB_PAGERIGHT:
            newLogicalArea = CRect(CPoint(Math::Min(logicalArea.left + logicalArea.GetSize().cx / 2, cadData.GetArea().right - logicalArea.GetSize().cx), logicalArea.top), logicalArea.GetSize());
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK   :
            newLogicalArea = CRect(CPoint(position, logicalArea.top), logicalArea.GetSize());
            break;
        default:
            return;
        }
        SetLogicalArea(newLogicalArea);
    }

    virtual void OnVScroll(UINT code, UINT position, CWnd* pScrollBar)
    {
        CRect newLogicalArea;
        switch (code) {
        case SB_TOP:
            newLogicalArea = CRect(CPoint(logicalArea.left, cadData.GetArea().top), logicalArea.GetSize());
            break;
        case SB_BOTTOM:
            newLogicalArea = CRect(CPoint(logicalArea.left, cadData.GetArea().bottom - logicalArea.GetSize().cy), logicalArea.GetSize());
            break;
        case SB_LINEUP:
            newLogicalArea = CRect(CPoint(logicalArea.left, Math::Max(logicalArea.top - logicalArea.GetSize().cy / 10, cadData.GetArea().top)), logicalArea.GetSize());
            break;
        case SB_LINEDOWN:
            newLogicalArea = CRect(CPoint(logicalArea.left, Math::Min(logicalArea.top + logicalArea.GetSize().cy / 10, cadData.GetArea().right - logicalArea.GetSize().cy)), logicalArea.GetSize());
            break;
        case SB_PAGEUP:
            newLogicalArea = CRect(CPoint(logicalArea.left, Math::Max(logicalArea.top - logicalArea.GetSize().cy / 2, cadData.GetArea().top)), logicalArea.GetSize());
            break;
        case SB_PAGEDOWN:
            newLogicalArea = CRect(CPoint(logicalArea.left, Math::Min(logicalArea.top + logicalArea.GetSize().cy / 2, cadData.GetArea().right - logicalArea.GetSize().cy)), logicalArea.GetSize());
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK   :
            newLogicalArea = CRect(CPoint(logicalArea.left, position), logicalArea.GetSize());
            break;
        default:
            return;
        }
        SetLogicalArea(newLogicalArea);
    }

    virtual LRESULT OnCommand(UINT notificationCode, int commandId)
    {
        if (commandId == editId && notificationCode == EN_KILLFOCUS) {
            int a = 1;
        }
        return 0;
    }

    virtual void OnUpdate(void* data)
    {
        if (data == nullptr)
            Invalidate();
        else
            OnUpdate(*static_cast<Figure*>(data));
    }

protected:
    virtual void OnClick(UINT keys, POINT point)
    {
        auto dc = DPtoLP(point);
        commandManager.OnClick(*dc, keys, point);
    }

    virtual void OnDragStart(POINT point)
    {
        auto dc = DPtoLP(point);
        commandManager.OnDragStart(*dc, point);
    }

    virtual void OnDragging(POINT point)
    {
        auto dc = DPtoLP(point);
        DebugOutput(_T("CadView::OnDragging"), point);
        commandManager.OnDragging(*dc, point);
    }

    virtual void OnDragEnd(POINT point)
    {
        auto dc = DPtoLP(point);
        commandManager.OnDragEnd(*dc, point);
    }

    virtual void OnDragStop()
    {
        CClientDC dc(*this);
        OnPrepareDC(dc);
        commandManager.OnDragStop(dc);
    }

private:
    void SetLogicalArea(const CRect& area)
    {
        logicalArea = area;
        logicalArea = logicalArea.Intersect(cadData.GetArea());
        SetScrollBar();
        ResetEditor ();
        Invalidate  ();
    }

    void ResetEditor()
    {
        CClientDC dc(*this);
        OnPrepareDC(dc);
        editor.Reset(dc);
    }

    void SetScrollBar()
    {
        SetHorizontalScrollBar();
        SetVerticalScrollBar  ();
        ShowScrollBar(SB_HORZ, logicalArea.GetSize().cx < cadData.GetArea().GetSize().cx);
        ShowScrollBar(SB_VERT, logicalArea.GetSize().cy < cadData.GetArea().GetSize().cy);
    }

    void SetHorizontalScrollBar()
    {
        SCROLLINFO scrollInfo;
        scrollInfo.cbSize = sizeof(SCROLLINFO);
        scrollInfo.fMask  = SIF_POS | SIF_RANGE | SIF_PAGE;
        scrollInfo.nMin   = cadData.GetArea().left ;
        scrollInfo.nMax   = cadData.GetArea().right;
        scrollInfo.nPos   = logicalArea.left;
        scrollInfo.nPage  = logicalArea.GetSize().cx;
        SetScrollInfo(SB_HORZ, &scrollInfo);
    }

    void SetVerticalScrollBar()
    {
        SCROLLINFO scrollInfo;
        scrollInfo.cbSize = sizeof(SCROLLINFO);
        scrollInfo.fMask  = SIF_POS | SIF_RANGE | SIF_PAGE;
        scrollInfo.nMin   = cadData.GetArea().top   ;
        scrollInfo.nMax   = cadData.GetArea().bottom;
        scrollInfo.nPos   = logicalArea.top;
        scrollInfo.nPage  = logicalArea.GetSize().cy;
        SetScrollInfo(SB_VERT, &scrollInfo);
    }

    void Enlarge(CRect& area, POINT basePoint, double rate)
    {
        area.Enlarge(basePoint, rate);
        //area = area.Intersect(cadData.GetArea());
    }

    unique_ptr<CDC> DPtoLP(POINT& point)
    {
        auto dc = unique_ptr<CDC>(new CClientDC(*this));
        OnPrepareDC(*dc);
        dc->DPtoLP(point);
        return dc;
    }

    //unique_ptr<CDC> LPtoDP(POINT& point)
    //{
    //    OnPrepareDC(dc);
    //    return dc.LPtoDP(point);
    //}

    void DrawPaper(CDC& dc)
    {
        dc.FillRect(cadData.GetArea(), paperColor);
        PenSelector selector(dc);
        dc.Rectangle(cadData.GetArea());
    }

    void OnUpdate(Figure& figure)
    {
        CClientDC dc(*this);
        OnPrepareDC(dc);
        auto      drawingBoundRect = figure.GetDrawingBoundRect(dc);
        dc.LPtoDP(drawingBoundRect);
        Invalidate(&drawingBoundRect);
    }

#ifdef _DEBUG
    void DebugOutput(tstring message, POINT point)
    {
        TCHAR text[256];
        _stprintf_s(text, _T("%s(x: %d, y: %d"), message.c_str(), point.x, point.y);
        DebugOutput(text);
    }

    void DebugOutput(tstring message)
    {
        ::SetWindowText(::GetParent(GetSafeHwnd()), message.c_str());
    }
#else // _DEBUG
#define DebugOutput(x)
#define DebugOutput(x, y)
#endif // _DEBUG
};

} // namespace CadCore

namespace Application {
using namespace CadCore;

class LineFigure : public Figure
{
    CLine position;

public:
    const CLine& Position() const
    {
        return position;
    }

    LineFigure(const LineFigure& figure)
        : Figure(figure), position(figure.position)
    {}

	LineFigure(const CLine& position) : position(position)
	{}

    virtual unique_ptr<Figure> Clone() const
    {
        return unique_ptr<Figure>(new LineFigure(*this));
    }

	virtual void DrawShape(CDC& dc)
	{
        dc.Draw(position);
	}

    virtual long GetDistance(CPoint point)
    {
        return position.GetDistance(point);
    }

    virtual vector<CPoint> GetPoints()
    {
        vector<CPoint> points;
        points.push_back(position.start);
        points.push_back(position.end  );
        return points;
    }
};

class RectangleFigure : public Figure
{
    CRect position;

public:
	const CRect& Position() const
	{
		return position;
	}

	RectangleFigure(const RECT& position) : position(position)
	{}

    RectangleFigure(const RectangleFigure& figure)
        : Figure(figure), position(figure.position)
    {}

    virtual unique_ptr<Figure> Clone() const
    {
        return unique_ptr<Figure>(new RectangleFigure(*this));
    }

    virtual void DrawShape(CDC& dc)
    {
		dc.Rectangle(Position());
	}

    virtual long GetDistance(CPoint point)
    {
        return position.GetDistance(point);
    }

    virtual CRect GetBoundRect()
    {
        return position;
    }

    virtual vector<CPoint> GetPoints()
    {
        return position.GetCorners();
    }
};

class EllipseFigure : public Figure
{
    CRect position;

public:
	const CRect& Position() const
	{
		return position;
	}

	EllipseFigure(const RECT& position) : position(position)
	{}

    EllipseFigure(const EllipseFigure& figure)
        : Figure(figure), position(figure.position)
    {}

    virtual unique_ptr<Figure> Clone() const
    {
        return unique_ptr<Figure>(new EllipseFigure(*this));
    }

    virtual void DrawShape(CDC& dc)
    {
		dc.Ellipse(Position());
	}

    virtual long GetDistance(CPoint point)
    {
        return CEllipse(position).GetDistance(point);
    }

    virtual CRect GetBoundRect()
    {
        return position;
    }

    virtual vector<CPoint> GetPoints()
    {
        vector<CPoint> points;
        points.push_back(position.GetCenter());
        points.push_back(CPoint::GetCenter(position.GetTopLeft    (), position.GetTopRight   ()));
        points.push_back(CPoint::GetCenter(position.GetTopRight   (), position.GetBottomRight()));
        points.push_back(CPoint::GetCenter(position.GetBottomRight(), position.GetBottomLeft ()));
        points.push_back(CPoint::GetCenter(position.GetBottomLeft (), position.GetTopLeft    ()));
        return points;
    }
};

class TextFigure : public Figure
{
    static  StandardLogFont logFont;
    CRect   position;
    tstring text;

public:
    CRect& Position()
    {
        return position;
    }

    tstring& Text()
    {
        return text;
    }

    TextFigure(const POINT& position, tstring text) : text(text)
    {
        this->position = CRect(position, CSize());
    }

    TextFigure(const TextFigure& figure)
        : Figure(figure), position(figure.position)
    {}

    virtual unique_ptr<Figure> Clone() const
    {
        return unique_ptr<Figure>(new TextFigure(*this));
    }

    void CalculateArea(CDC& dc)
    {
        FontSelector fontSelector(dc, logFont);
        dc.DrawText(text, position, DT_LEFT | DT_TOP | DT_CALCRECT);
    }

    virtual void DrawShape(CDC& dc)
    {
        CalculateArea(dc);

        FontSelector fontSelector(dc, logFont);
        dc.SetTextColor(GetColor());
        dc.SetBkMode(TRANSPARENT);
        dc.DrawText(text, position, DT_LEFT | DT_TOP);
    }

    virtual long GetDistance(CPoint point)
    {
        return position.GetDistance(point);
    }

    virtual CRect GetBoundRect()
    {
        return position;
    }

    virtual vector<CPoint> GetPoints()
    {
        return position.GetCorners();
    }
};

StandardLogFont TextFigure::logFont;

class AddLineCommand : public AddCommand
{
public:
	AddLineCommand(CadData& cadData, CadView& cadView) : AddCommand(cadData, cadView)
	{}

protected:
	virtual unique_ptr<Figure> CreateFigure(POINT point) const
	{
		return unique_ptr<Figure>(new LineFigure(CLine(firstPoint, point)));
	}
};

class AddRectangleCommand : public AddCommand
{
public:
	AddRectangleCommand(CadData& cadData, CadView& cadView) : AddCommand(cadData, cadView)
	{}

protected:
	virtual unique_ptr<Figure> CreateFigure(POINT point) const
	{
		return unique_ptr<Figure>(new RectangleFigure(CRect(firstPoint, point)));
	}
};

class AddEllipseCommand : public AddCommand
{
public:
	AddEllipseCommand(CadData& cadData, CadView& cadView) : AddCommand(cadData, cadView)
	{}

protected:
	virtual unique_ptr<Figure> CreateFigure(POINT point) const
	{
		return unique_ptr<Figure>(new EllipseFigure(CRect(firstPoint, point)));
	}
};

class AddCircleCommand : public AddCommand
{
public:
	AddCircleCommand(CadData& cadData, CadView& cadView) : AddCommand(cadData, cadView)
	{}

protected:
	virtual unique_ptr<Figure> CreateFigure(POINT point) const
	{
		auto radius = CPoint(firstPoint).GetDistance(point);
		return unique_ptr<Figure>(new EllipseFigure(MakeRectangle(CPoint(firstPoint), radius)));
	}

	virtual void OnDrawRubberBand(CDC& dc, POINT point)
	{
		AddCommand::OnDrawRubberBand(dc, point);
		dc.Ellipse(MakeRectangle(CPoint(firstPoint), 2));
	}

private:
	static CRect MakeRectangle(CPoint centerPoint, double radius)
	{
		return CRect(centerPoint - CSize(int(radius), int(radius)),
			         centerPoint + CSize(int(radius), int(radius)));
	}
};


class AddTextCommand : public Command
{
public:
    AddTextCommand(CadData& cadData, CadView& cadView) : Command(cadData, cadView)
    {}

    virtual void OnClick(CDC& dc, UINT keys, POINT point)
    {
        tstringstream text;
        text << GetTickCount();
        auto figure = new TextFigure(point, text.str());

        figure->CalculateArea(dc);
        cadView.SetEdit(figure->Text(), StandardLogFont::defaultHeight, figure->Position());
        cadData.Add(unique_ptr<Figure>(figure));
    }
};

class MainWindow : public CWnd
{
	static const _TCHAR title[];

	CadData		   cadData;
	CadView		   cadView;
	CommandManager commandManager;

public:
	MainWindow(HINSTANCE hInstance)
		: CWnd(hInstance), commandManager(cadData, cadView), cadView(hInstance, cadData, commandManager)
	{}

	bool Create(int nCmdShow)
	{
		if (CWnd::Create(nullptr, title, WS_OVERLAPPEDWINDOW, IDC_MiniCad32, nCmdShow)) {
			cadView.Create(this);
			AdjustViewSize();
			return true;
		}
		return false;
	}

protected:
	virtual LRESULT OnCommand(UINT notificationCode, int commandId)
	{
		switch (commandId)
		{
        case ID_EDIT_UNDO:
            cadData.Undo();
            break;
        case ID_EDIT_REDO:
            cadData.Redo();
            break;

        case ID_VISUAL_HOME:
            cadView.Home();
            break;

        case ID_FIGURE_SELECT:
            commandManager.SetCommand(unique_ptr<Command>(new SelectCommand(cadData, cadView)));
            break;
        case ID_FIGURE_DELETE:
            cadData.Delete();
            break;

        case ID_FIGURE_LINE:
			commandManager.SetCommand(unique_ptr<Command>(new AddLineCommand(cadData, cadView)));
			break;
		case ID_FIGURE_RECTANGLE:
			commandManager.SetCommand(unique_ptr<Command>(new AddRectangleCommand(cadData, cadView)));
			break;
		case ID_FIGURE_ELLIPSE:
			commandManager.SetCommand(unique_ptr<Command>(new AddEllipseCommand(cadData, cadView)));
			break;
		case ID_FIGURE_CIRCLE:
			commandManager.SetCommand(unique_ptr<Command>(new AddCircleCommand(cadData, cadView)));
			break;
        case ID_FIGURE_TEXT:
            commandManager.SetCommand(unique_ptr<Command>(new AddTextCommand(cadData, cadView)));
            break;

        case ID_COLOR_BLACK:
			cadData.SetCurrentColor(Color::Black);
			break;
		case ID_COLOR_RED:
			cadData.SetCurrentColor(Color::Red  );
			break;
		case ID_COLOR_GREEN:
			cadData.SetCurrentColor(Color::Green);
			break;
		case ID_COLOR_BLUE:
			cadData.SetCurrentColor(Color::Blue );
			break;

		case IDM_ABOUT:
			::DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		}
		return 0;
	}

	virtual void OnDestroy()
	{
		::PostQuitMessage(0);
	}

	virtual void OnSize()
	{
		AdjustViewSize();
	}

private:
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(lParam);
		switch (message)
		{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				::EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}
		return (INT_PTR)FALSE;
	}

	void AdjustViewSize()
	{
		const auto clientArea = GetClientArea();
		cadView.Move(clientArea);
	}
};

const _TCHAR MainWindow::title[] = _T("MiniCad32");

class Program
{
public:
	int Main(HINSTANCE hInstance, int nCmdShow)
	{
		return MainWindow(hInstance).Create(nCmdShow) ? MainMessageLoop() : FALSE;
	}

private:
	static int MainMessageLoop()
	{
		MSG msg;
		while (::GetMessage(&msg, nullptr, 0, 0)) {
			::TranslateMessage(&msg);
			::DispatchMessage (&msg);
		}
		return int(msg.wParam);
	}
};

} // namespace Application

} // namespace MiniCad
} // namespace Shos

int APIENTRY _tWinMain(_In_     HINSTANCE hInstance	   ,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_     LPWSTR    lpCmdLine    ,
                       _In_     int       nCmdShow	   )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //InitializeMemoryLeakDetector();
	auto result = Shos::MiniCad::Application::Program().Main(hInstance, nCmdShow);
    return result;
}
