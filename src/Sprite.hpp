/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sprite.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/17 18:56:01 by nathan            #+#    #+#             */
/*   Updated: 2023/02/02 11:44:53 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SPRITE_CLASS_H
# define SPRITE_CLASS_H

/*
** Sprites implementation
** It search in OAM entry and manage the sprite drawing for Ppu.
*/

#include <array>
#include <vector>

struct OAM_entry
{
public:
	unsigned char posY;
	unsigned char posX;
	unsigned char tileIndex;
	unsigned char attributes;

	bool getBGOverWindow() const;
	bool getFlipY() const;
	bool getFlipX() const;
	bool getDMGPalette() const;
	bool getTileVramBank() const;
	unsigned char getCGBPalette() const;

	friend bool operator==(const OAM_entry &a, const OAM_entry &b) {
		return ((a.posX == b.posX) && (a.posY == b.posY) && (a.tileIndex == b.tileIndex) && (a.attributes == b.attributes));
	}
};

class Sprite
{

public:
	Sprite(OAM_entry newOAM, unsigned char newSpriteHeight);
	~Sprite(void);

	unsigned char getSpriteHeight() const {return spriteHeight;}
	unsigned long getPaletteValue() const;
	std::array<short, 8> getLineColorCode(int y) const { return data[y]; }
	std::array<short, 8> getColoredLine(int y) const;
	void flipY();
	void flipX();
private:
	unsigned char	spriteHeight;
	std::vector<std::array<short, 8>>	data;

	OAM_entry OAM_Data;
};

#endif
