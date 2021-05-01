#pragma once

enum Align
{
	Left,
	Right,
	Center
};

class Text
{
public:
	Text() noexcept(false);

	~Text()
	{
	};

	void Print(std::string text, DirectX::SimpleMath::Vector2 position, Align align,
	           DirectX::XMVECTORF32 color = DirectX::Colors::White);

	void CreateDeviceDependentResources();
	void SetViewport(float width, float height);

private:
	std::unique_ptr<DirectX::SpriteFont> m_font;
	std::unique_ptr<DirectX::SpriteBatch> m_batch;
	std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
};
