/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mem.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nallani <nallani@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/07 20:49:02 by nallani           #+#    #+#             */
/*   Updated: 2023/02/02 10:17:29 by nallani          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef MEM_CLASS_H
# define MEM_CLASS_H

/*
** Memory
** This module handle the memory
** We use an array with operator overload for taking some action if the
** program write in special register.
** Handle read-only bits
** Use MBC for handle bank switching
*/

#include <string>
#include <map>
#include <vector>
#include <array>
#include "MBC.hpp"

#define HDMA1 0xFF51
#define HDMA2 0xFF52
#define HDMA3 0xFF53
#define HDMA4 0xFF54
#define HDMA5 0xFF55
#define MEM_SIZE (0xFFFF + 1)
#define ROM_BANK_SIZE (16 * 1024)

class Mem;

class MemWrap {
	public:
		MemWrap(unsigned short offset, unsigned char& ref):
			addr(offset), value(ref) {}
		unsigned char& operator=(unsigned char newValue);
		unsigned char	operator+() {return value;}
		unsigned char* operator&() {return &value;}
		operator unsigned char&() { return value; }
		operator const unsigned char&() const { return value; }
	private:
		const unsigned short addr;
		unsigned char& value;
};


class Mem {
public:
	Mem() = delete;
	Mem(const std::string& pathToRom);
	static Mem* loadFromFile(const std::string& pathToRom);
	virtual ~Mem();
	MemWrap operator[](unsigned int i);
	const MemWrap operator[](unsigned int i) const;
	void supervisorWrite(unsigned int addr, unsigned char value);
	void saveByte(unsigned int addr, unsigned char value);
	unsigned char* getVram() {return &internalArray[0x8000];}
	static std::vector<unsigned char> readFile(const std::string& filename);

	virtual unsigned char* getCGBVram() {throw("");}
	virtual const std::array<unsigned char, 64>& getBGPalettes(){throw("");}// should never be called
	virtual const std::array<unsigned char, 64>& getOBJPalettes(){throw("");}// should never be called

	bool isValid;
	std::string getTitle();
	bool	isCGB();
	int		getCartridgeType();

	unsigned char *getInternalArray() {return internalArray; }

	static const std::map<unsigned short, unsigned char> readOnlyBits;
    std::vector<unsigned char*>	extraRamBanks;
	unsigned char*	builtinExtraRam;
    std::vector<unsigned char*>	romBanks;
	MBC* mbc;
	void saveByteInSave(unsigned short addr, unsigned char value) const;
	unsigned char extraRamBanksNb;

	unsigned short getRamBankSize() const;

protected:
	void init();
	unsigned char*	internalArray;

	virtual unsigned char&	getRefWithBanks(unsigned short addr) const;
	static int 			getRomBanksNb(char romSizeCode);
	static int 			getExtraRamBanksNb(char ramSizeCode);
};

class CGBMem : public Mem{
public:
	CGBMem(const std::string& pathToRom);
	virtual ~CGBMem();
	virtual unsigned char* getCGBVram() {return CGBVramBank;}

	mutable bool			bIsUsingCGBVram;
	mutable unsigned char	CGBextraRamBankNb;
	virtual const std::array<unsigned char, 64>& getBGPalettes() {return BGPalettes;}
	virtual const std::array<unsigned char, 64>& getOBJPalettes() {return OBJPalettes;}
	mutable std::array<unsigned char, 64> BGPalettes;
	mutable std::array<unsigned char, 64> OBJPalettes;

	// CGB Compatibility palette saving
	mutable std::array<unsigned char, 8> CGBCompatPaletteSaveBG;
	mutable std::array<unsigned char, 16> CGBCompatPaletteSaveOBJ;

	std::array<unsigned char*, 8> getCGBExtraRamBanks() {return CGBextraRamBanks;}
protected:
	virtual unsigned char&	getRefWithBanks(unsigned short addr) const;
	unsigned char*	CGBVramBank;
	std::array<unsigned char*, 8> CGBextraRamBanks;
};

#endif
