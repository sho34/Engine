#include "pch.h"
#include "String2D.h"
#include "../Common/DirectXHelper.h"

std::map<std::wstring, String2DPtr> ui2DStrings;

namespace UI {

  auto InitializeString2D(auto& d2d1DeviceContext, auto& dWriteFactory, auto& string2D) {
    DX::ThrowIfFailed(d2d1DeviceContext->CreateSolidColorBrush(string2D->stringDefinition.fontColor, &string2D->textBrush));
    DX::ThrowIfFailed(dWriteFactory->CreateTextFormat(string2D->stringDefinition.fontFamilyName.c_str(), NULL, string2D->stringDefinition.fontWeight, string2D->stringDefinition.fontStyle, string2D->stringDefinition.fontStretch, string2D->stringDefinition.fontSize, string2D->stringDefinition.locale.c_str(), &string2D->textFormat));
    DX::ThrowIfFailed(string2D->textFormat->SetTextAlignment(string2D->stringDefinition.textAlignment));
    DX::ThrowIfFailed(string2D->textFormat->SetParagraphAlignment(string2D->stringDefinition.paragraphAlignment));
    string2D->drawRect = string2D->stringDefinition.drawRect;
  }

  std::shared_ptr<String2D> CreateUI2DString(std::wstring stringName, CComPtr<ID2D1DeviceContext5>& d2d1DeviceContext, CComPtr<IDWriteFactory>& dWriteFactory, String2DDefinition def)
  {
    String2DPtr string2D = std::make_shared<String2DT>();

    string2D->stringDefinition = def;
    string2D->text = def.text;

    InitializeString2D(d2d1DeviceContext, dWriteFactory, string2D);

    ui2DStrings[stringName] = string2D;

    return string2D;
  }

  void DestroyUI2DStrings()
  {
    for (auto& [name, string2D] : ui2DStrings)
    {
      string2D->textBrush.Release();
      string2D->textBrush = nullptr;
      string2D->textFormat.Release();
      string2D->textFormat = nullptr;
      string2D = nullptr;
    }
    ui2DStrings.clear();
  }

  std::shared_ptr<String2D> GetUI2DString(std::wstring stringName)
  {
    return ui2DStrings[stringName];
  }

  void DrawUI2DStrings(CComPtr<ID2D1DeviceContext5> d2d1DeviceContext)
  {
    if (ui2DStrings.size() == 0) return;

    d2d1DeviceContext->BeginDraw();
    d2d1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

    for (auto& [name, string2D] : ui2DStrings)
    {
      d2d1DeviceContext->DrawText(string2D->text.c_str(), static_cast<UINT32>(string2D->text.length()), string2D->textFormat, &string2D->drawRect, string2D->textBrush);
    }

    /*
    D2D1_RECT_F Wnd{};

    Wnd = D2D1::RectF(x, y, x + width, y + height);

    d2d1DeviceContext->DrawText(label.c_str(), static_cast<UINT32>(label.length()), textFormat.p, &Wnd, textBrush.p);
    */

    DX::ThrowIfFailed(d2d1DeviceContext->EndDraw());
  }

}

