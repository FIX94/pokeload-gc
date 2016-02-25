/*
Copyright (c) 2015, TuxSH
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of pkmgchax nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "stdafx.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <LibPkmGC/Colosseum/SaveEditing/Save.h>
#include <LibPkmGC/XD/SaveEditing/Save.h>

#include <string>
#include <cstring>

using namespace LibPkmGC;

// 0 = unknown 
u32 colosseumInBattleAddrs[] = { 0x8045A008, 0x8046E928, 0x804BBDA8 };
u32 XDInBattleAddrs[] = { 0x8047F648, 0x804A2128, 0x804DC708 };

u32 colosseumJumpToPCDataCodeJAP[] = {
	/*
	lwz       r3, -0x5A98(r13)
	addi      r3, r3, 0xB88
	mtctr	  r3
	bctrl
	*/
	0x806DA568, 0x38630B88, 0x7C6903A6, 0x4E800421
};

u32 colosseumJumpToPCDataCodeNTSC[] = {
	/*
	lwz       r3, -0x5A68(r13)
	addi      r3, r3, 0xB88
	mtctr	  r3
	bctrl
	*/
	0x806DA598, 0x38630B88, 0x7C6903A6, 0x4E800421
};

u32 colosseumJumpToPCDataCodePAL[] = {
	/*
	lwz       r3, -0x5A58(r13)
	addi      r3, r3, 0xB88
	mtctr	  r3
	bctrl
	*/
	0x806DA5A8, 0x38630B88, 0x7C6903A6, 0x4E800421
};

u32 XDJumpToPCDataCodeJAP[] = {
	/*
	lwz       r3, -0x4758(r13)
	addi      r3, r3, 0xAC0
	mtctr	  r3
	bctrl
	*/
	0x806DB8A8, 0x38630AC0, 0x7C6903A6, 0x4E800421
};

u32 XDJumpToPCDataCodeNTSC[] = {
	/*
	lwz       r3, -0x4728(r13)
	addi      r3, r3, 0xAD0
	mtctr	  r3
	bctrl
	*/
	0x806DB8D8, 0x38630AD0, 0x7C6903A6, 0x4E800421
};

u32 XDJumpToPCDataCodePAL[] = {
	/*
	lwz       r3, -0x4720(r13)
	addi      r3, r3, 0xAD0
	mtctr	  r3
	bctrl
	*/
	0x806DB8E0, 0x38630AD0, 0x7C6903A6, 0x4E800421
};

u32 *colosseumJumpToPCDataCode[] = {
	colosseumJumpToPCDataCodeJAP, colosseumJumpToPCDataCodeNTSC, colosseumJumpToPCDataCodePAL
};

u32 *XDJumpToPCDataCode[] = {
	XDJumpToPCDataCodeJAP, XDJumpToPCDataCodeNTSC, XDJumpToPCDataCodePAL
};

inline size_t getFileSize(FILE* f){
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	rewind(f);
	return fsize;
}

GC::SaveEditing::Save* readSaveFile(const char* fname){
	FILE* f = fopen(fname, "rb");
	if (f == NULL) return NULL;

	size_t fsize = getFileSize(f);

	GC::SaveEditing::Save* ret = NULL;
	if (fsize == 0x60040){
		u8* data = new u8[0x60040];
		fread(data, 1, 0x60040, f);
		ret = new Colosseum::SaveEditing::Save(data, true);
		delete[] data;
	}
	else if (fsize == 0x56040){
		u8* data = new u8[0x56040];
		fread(data, 1, 0x56040, f);
		ret = new XD::SaveEditing::Save(data, true);
		delete[] data;
	}

	fclose(f);

	return ret;
}

void parseGCIData(u8* data, bool& isXD, RegionIndex& region){
	isXD = false;
	region = NoRegion;

	if (memcmp(data + 4, "01", 2) != 0 || data[0] != 'G') return;

	if (memcmp(data + 1, "C6", 2) == 0) isXD = false;
	else if (memcmp(data + 1, "XX", 2) == 0) isXD = true;
	else return;

	switch (data[3]){
	case 'J': region = NTSC_J; break;
	case 'E': region = NTSC_U; break;
	case 'P': region = PAL; break;
	default: break;
	}

}

int _tmain(int argc, _TCHAR* argv[]){
	GC::SaveEditing::Save* save = readSaveFile("save.gci");
	if (save == NULL){
		std::cerr << "save.gci was not found or is invalid." << std::endl;
		return -1;
	}

	bool isXD;
	RegionIndex region;

	parseGCIData(save->GCIData, isXD, region);

	if (region == NoRegion || isXD != LIBPKMGC_IS_XD(SaveEditing::Save, save)){
		std::cerr << "Invalid save.gci" << std::endl;
		return -2;
	}

	FILE* payload_f = fopen("payload.bin", "rb");

	if (payload_f == NULL){
		std::cerr << "payload.bin was not found." << std::endl;
		return -4;
	}

	GC::SaveEditing::SaveSlot* sl = save->getMostRecentSlot();

	size_t payload_sz = getFileSize(payload_f);
	if (payload_sz > sl->PC->fixedSize){
		std::cerr << "payload.bin is too big (max.: " << sl->PC->fixedSize << " bytes)." << std::endl;
		fclose(payload_f);
		return -5;
	}

	u8* payload = new u8[payload_sz];
	fread(payload, 1, payload_sz, payload_f);
	fclose(payload_f);


	std::string name0 = std::string("NINJA") + std::string(124 - 5, 'A');
	GC::PokemonString name(name0.c_str());

	u32 LR_value = (isXD) ? (XDInBattleAddrs[region - 1] + 0x4e + 76 + 4 + 2) : (colosseumInBattleAddrs[region - 1] + 0x2e + 124 + 4 + 2);

	// Pointer to the data where the name of the 1st party Pokémon is stored
	u8* firstPkmName = NULL;

	if (isXD){
		XD::SaveEditing::SaveSlot* sl_XD = (XD::SaveEditing::SaveSlot*) sl;
		firstPkmName = sl_XD->data + 0xa8 + sl_XD->substructureOffsets[1] + 0x30 + 0x4e;
		name.save(firstPkmName, 76 / 2);
		IntegerManip::BE::fromInteger<u32, u8*>(firstPkmName + 76, LR_value);
		IntegerManip::BE::fromArrayOfIntegers<u32*, u8*>(firstPkmName + 82, XDJumpToPCDataCode[region - 1], XDJumpToPCDataCode[region - 1] + 4);

		std::copy(payload, payload + payload_sz, sl_XD->data + 0xa8 + sl_XD->substructureOffsets[2]); // PC Data
	}

	else{
		firstPkmName = sl->data + 0x78 + 0x30 + 0x2e;
		name.save(firstPkmName, 124 / 2);
		IntegerManip::BE::fromInteger<u32, u8*>(firstPkmName + 124, LR_value);
		IntegerManip::BE::fromArrayOfIntegers<u32*, u8*>(firstPkmName + 130, colosseumJumpToPCDataCode[region - 1], colosseumJumpToPCDataCode[region - 1] + 4);

		std::copy(payload, payload + payload_sz, sl->data + 0x78 + 0xb18); // PC Data
	}

	delete[] payload;

	size_t outsz = save->fixedSize + 0x40;
	u8* outbuf = new u8[outsz];
	save->saveEncrypted(outbuf, true, false);
	delete save;

	FILE* out_file = fopen("save_out.gci", "wb+");
	if (out_file == NULL){
		std::cerr << "Could not write to save_out.bin." << std::endl;
		return -7;
	}

	fwrite(outbuf, 1, outsz, out_file);
	fclose(out_file);

	return 0;
}
