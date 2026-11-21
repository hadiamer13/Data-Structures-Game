// gui/widgets/NineSlice.cpp
#include "NineSlice.h"

namespace
{
    // Border ko texture ke andar rakhta hai aur beech mein kam az kam 1 pixel chhodta hai.
    int clampBorder(int border, unsigned texSize)
    {
        const int limit = (static_cast<int>(texSize) - 1) / 2;
        int result = border;
        if (result > limit)
        {
            result = limit;
        }
        if (result < 1)
        {
            result = 1;
        }
        return result;
    }

    float minF(float a, float b)
    {
        return (a < b) ? a : b;
    }
}

NineSlice::NineSlice()
    : m_tex(nullptr), m_border(0), m_color(sf::Color::White)
{
}

// Texture set karta hai. Texture chhota ho to frame use nahi hota.
void NineSlice::setTexture(const sf::Texture* tex, int border)
{
    m_tex = nullptr;
    m_border = border;
    if (tex != nullptr && tex->getSize().x >= 3 && tex->getSize().y >= 3)
    {
        m_tex = tex;
    }
    rebuild();
}

void NineSlice::setRect(const sf::FloatRect& outer)
{
    m_rect = outer;
    rebuild();
}

// Rang sab 9 hisson par lagata hai.
void NineSlice::setColor(const sf::Color& c)
{
    m_color = c;
    for (int i = 0; i < 9; ++i)
    {
        m_part[i].setColor(c);
    }
}

bool NineSlice::hasTexture() const
{
    return m_tex != nullptr;
}

// 9 hisson ka source rectangle aur screen par destination calculate karta hai.
void NineSlice::rebuild()
{
    if (m_tex == nullptr)
    {
        return;
    }

    const sf::Vector2u ts = m_tex->getSize();
    const int tw = static_cast<int>(ts.x);
    const int th = static_cast<int>(ts.y);
    const int bx = clampBorder(m_border, ts.x);
    const int by = clampBorder(m_border, ts.y);
    const int b = (bx < by) ? bx : by;     // sab corners ke liye aik hi border size
    const float bf = static_cast<float>(b);

    // Frame chhota ho to kone bhi chhote ho jate hain, warna apna size rakhte hain.
    const float k = minF(1.f, minF(m_rect.width / (2.f * bf), m_rect.height / (2.f * bf)));
    const float cw = bf * k;                // screen par kone ya kinare ka size
    const float midW = m_rect.width - 2.f * cw;
    const float midH = m_rect.height - 2.f * cw;

    const int srcX[3] = {0, b, tw - b};
    const int srcW[3] = {b, tw - 2 * b, b};
    const int srcY[3] = {0, b, th - b};
    const int srcH[3] = {b, th - 2 * b, b};

    const float dstX[3] = {m_rect.left, m_rect.left + cw, m_rect.left + cw + midW};
    const float dstY[3] = {m_rect.top, m_rect.top + cw, m_rect.top + cw + midH};
    const float dstW[3] = {cw, midW, cw};
    const float dstH[3] = {cw, midH, cw};

    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            sf::Sprite& s = m_part[row * 3 + col];
            s.setTexture(*m_tex);
            s.setTextureRect(sf::IntRect(srcX[col], srcY[row], srcW[col], srcH[row]));
            s.setPosition(dstX[col], dstY[row]);
            s.setScale(dstW[col] / static_cast<float>(srcW[col]),
                       dstH[row] / static_cast<float>(srcH[row]));
            s.setColor(m_color);
        }
    }
}

// Saare 9 hisse window par draw karta hai.
void NineSlice::draw(sf::RenderWindow& window) const
{
    if (m_tex == nullptr)
    {
        return;
    }
    for (int i = 0; i < 9; ++i)
    {
        window.draw(m_part[i]);
    }
}