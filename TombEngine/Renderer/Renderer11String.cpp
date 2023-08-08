#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Specific/trutils.h"

namespace TEN::Renderer
{
	void Renderer11::AddDebugString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags, RendererDebugPage page)
	{
		constexpr auto FLAGS = PRINTSTRING_OUTLINE | PRINTSTRING_CENTER;

		if (DebugPage != page)
			return;

		AddString(string, pos, color, scale, FLAGS);
	}

	void Renderer11::AddString(int x, int y, const std::string& string, D3DCOLOR color, int flags)
	{
		AddString(string, Vector2(x, y), Color(color), 1.0f, flags);
	}

	void Renderer11::AddString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags)
	{
		constexpr auto BLINK_VALUE_MAX = 1.0f;
		constexpr auto BLINK_VALUE_MIN = 0.1f;
		constexpr auto BLINK_TIME_STEP = 0.2f;

		if (m_Locked)
			return;

		if (string.empty())
			return;

		try
		{
			auto screenRes = GetScreenResolution();
			auto windowRes = GetWindowResolution();
			auto factor = Vector2(screenRes.x / SCREEN_SPACE_RES.x, screenRes.y / SCREEN_SPACE_RES.y);
			float uiScale = (screenRes.x > screenRes.y) ? factor.y : factor.x;
			float fontSpacing = m_gameFont->GetLineSpacing();
			float fontScale = REFERENCE_FONT_SIZE / fontSpacing;

			float virtualScreenAspect = static_cast<float>(SCREEN_SPACE_RES.x) / static_cast<float>(SCREEN_SPACE_RES.y);
			float screenAspect = static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight);
			float windowAspect = static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);

			// Calculate the offset and scale of the text based on the aspect ratio
			auto aspectRatioCorrection = Vector2();

			auto rescaledPos = Vector2(pos.x / SCREEN_SPACE_RES.x, pos.y / SCREEN_SPACE_RES.y);
			auto screenOffset = Vector2();

			if (screenAspect > windowAspect)
			{
				// Bar Top
				aspectRatioCorrection.x = 0;
				aspectRatioCorrection.y = windowAspect / screenAspect;

				screenOffset.x = 0;
				screenOffset.y = (1.0 - aspectRatioCorrection.y) * windowRes.y;

				uiScale *= ((float)m_windowWidth / (float)m_screenWidth);
			}
			else
			{
				// Bar Side
				aspectRatioCorrection.x = screenAspect / windowAspect;
				aspectRatioCorrection.y = 0;

				screenOffset.x = (1.0 - aspectRatioCorrection.x) * windowRes.x;
				screenOffset.y = 0;

				uiScale *= ((float)m_windowHeight / (float)m_screenHeight);
			}

			auto aspectOffsetStart = Vector2(
				((screenOffset.x - (screenOffset.x * 0.5))),
				((screenOffset.y - (screenOffset.y * 0.5)))
			);
			auto aspectOffsetEnd = Vector2(
				m_windowWidth - ((screenOffset.x - (screenOffset.x * 0.5))),
				m_windowHeight - ((screenOffset.y - (screenOffset.y * 0.5)))
			);

			auto aspectOffset = Vector2(
				Lerp(aspectOffsetStart.x, aspectOffsetEnd.x, rescaledPos.x),
				Lerp(aspectOffsetStart.y, aspectOffsetEnd.y, rescaledPos.y)
			);
			
			auto stringLines = SplitString(string);
			float yOffset = 0.0f;
			for (const auto& line : stringLines)
			{
				// Prepare structure for renderer.
				RendererStringToDraw rString;
				rString.String = TEN::Utils::ToWString(line);
				rString.Flags = flags;
				rString.X = 0;
				rString.Y = 0;
				rString.Color = color.ToVector3();
				rString.Scale = (uiScale * fontScale) * scale;

				// Measure string.
				auto size = Vector2(m_gameFont->MeasureString(rString.String.c_str())) * rString.Scale;

				rString.X = (flags & PRINTSTRING_CENTER) ? aspectOffset.x - (size.x / 2.0f) : aspectOffset.x;
				rString.Y = aspectOffset.y + yOffset;

				if (flags & PRINTSTRING_BLINK)
				{
					rString.Color *= BlinkColorValue;

					if (!IsBlinkUpdated)
					{
						// Calculate blink increment based on sine wave.
						BlinkColorValue = ((sin(BlinkTime) + BLINK_VALUE_MAX) * 0.5f) + BLINK_VALUE_MIN;

						// Update blink time.
						BlinkTime += BLINK_TIME_STEP;
						if (BlinkTime > PI_MUL_2)
							BlinkTime -= PI_MUL_2;

						IsBlinkUpdated = true;
					}
				}

				yOffset += size.y;
				m_strings.push_back(rString);
			}
		}
		catch (std::exception& ex)
		{
			TENLog(std::string("Unable to process string: '") + string + "'. Exception: " + std::string(ex.what()), LogLevel::Error);
		}
	}

	void Renderer11::DrawAllStrings()
	{
		float shadowOffset = 1.5f / (REFERENCE_FONT_SIZE / m_gameFont->GetLineSpacing());

		m_spriteBatch->Begin();

		for (const auto& rString : m_strings)
		{
			// Draw shadow.
			if (rString.Flags & PRINTSTRING_OUTLINE)
			{
				m_gameFont->DrawString(
					m_spriteBatch.get(), rString.String.c_str(),
					Vector2(rString.X + shadowOffset * rString.Scale, rString.Y + shadowOffset * rString.Scale),
					Vector4(0.0f, 0.0f, 0.0f, 1.0f) * ScreenFadeCurrent,
					0.0f, Vector4::Zero, rString.Scale);
			}

			// Draw string.
			m_gameFont->DrawString(
				m_spriteBatch.get(), rString.String.c_str(),
				Vector2(rString.X, rString.Y),
				Vector4(rString.Color.x, rString.Color.y, rString.Color.z, 1.0f) * ScreenFadeCurrent,
				0.0f, Vector4::Zero, rString.Scale);
		}

		m_spriteBatch->End();

		IsBlinkUpdated = false;
		m_strings.clear();
	}
}
