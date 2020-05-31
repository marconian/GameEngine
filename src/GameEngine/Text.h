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
    ~Text() { };

    void Print(std::string text, Vector2 position, Align align, DirectX::XMVECTORF32 color = Colors::White);

    void CreateDeviceDependentResources();
    void SetViewport(const float width, const float height);

private:
    std::unique_ptr<DirectX::SpriteFont>            m_font;
    std::unique_ptr<DirectX::SpriteBatch>           m_batch;
    std::unique_ptr<DirectX::DescriptorHeap>        m_resourceDescriptors;
};

