#pragma once

using namespace DirectX;

namespace UI {

	struct String2DDefinition {
		std::wstring text = L"";
		D2D1::ColorF fontColor = D2D1::ColorF::White;
		std::wstring fontFamilyName = L"Verdana";
		std::wstring locale = L"en_US";
		FLOAT fontSize = 12.0f;
		DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL;
		DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL;
		DWRITE_FONT_STRETCH fontStretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL;
		DWRITE_TEXT_ALIGNMENT textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
		DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
		D2D1_RECT_F drawRect = {};
	};

	struct String2D
	{
		String2DDefinition stringDefinition;
		CComPtr<ID2D1SolidColorBrush> textBrush;
		CComPtr<IDWriteTextFormat> textFormat;
		D2D1_RECT_F drawRect = {};
		std::wstring text;
	};

	std::shared_ptr<String2D> CreateUI2DString(std::wstring stringName, CComPtr<ID2D1DeviceContext5>& d2d1DeviceContext, CComPtr<IDWriteFactory>& dWriteFactory, String2DDefinition = {});
	void DestroyUI2DStrings();
	std::shared_ptr<String2D> GetUI2DString(std::wstring stringName);
	void DrawUI2DStrings(CComPtr<ID2D1DeviceContext5> d2d1DeviceContext);
}

typedef UI::String2D String2DT;
typedef std::shared_ptr<String2DT> String2DPtr;

