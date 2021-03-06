/*
    Aiftool
    Copyright 2007 Martin Storsjo

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Martin Storsjo
    martin@martin.st
*/

/*
This is a quick and dirty hack, to emulate the real aiftool.exe by
constructing a aif spec file and calling genaif with that one.
*/

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

uint32_t readUint32(FILE* rsc) {
	uint8_t buf[4];
	fread(buf, 1, 4, rsc);
	return buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24);
}

uint16_t readUint16(FILE* rsc) {
	uint8_t buf[2];
	fread(buf, 1, 2, rsc);
	return buf[0] | (buf[1]<<8);
}

static const char *langs[] = {
	"ELangTest",
	"ELangEnglish",
	"ELangFrench",
	"ELangGerman",
	"ELangSpanish",
	"ELangItalian",
	"ELangSwedish",
	"ELangDanish",
	"ELangNorwegian",
	"ELangFrscnish",
	"ELangAmerican",
	"ELangSwissFrench",
	"ELangSwissGerman",
	"ELangPortuguese",
	"ELangTurkish",
	"ELangIcelandic",
	"ELangRussian",
	"ELangHungarian",
	"ELangDutch",
	"ELangBelgianFlemish",
	"ELangAustralian",
	"ELangBelgianFrench",
	"ELangAustrian",
	"ELangNewZealand",
	NULL
};

int main(int argc, char *argv[]) {
	const char* dir = argv[1];
	char rscfile[PATH_MAX], specfile[PATH_MAX];
	sprintf(rscfile, "%saif.rsc", dir);
	FILE* rsc = fopen(rscfile, "rb");
	if (!rsc) {
		perror(rscfile);
		return 1;
	}
	sprintf(specfile, "%saif.spec", dir);
	FILE* spec = fopen(specfile, "w");
	if (!spec) {
		perror(specfile);
		return 1;
	}

	uint8_t numLanguages = 0;
	while (langs[numLanguages])
		numLanguages++;

	fseek(rsc, 4, SEEK_CUR);
	uint32_t uid = readUint32(rsc);
	/*uint16_t numIcons =*/ readUint16(rsc);
	uint16_t numCaptions = readUint16(rsc);
	for (uint16_t i = 0; i < numCaptions; i++) {
		uint8_t code = fgetc(rsc);
		uint8_t len = fgetc(rsc);
		char* caption = new char[len+1];
		for (uint8_t j = 0; j < len; j++) {
			caption[j] = fgetc(rsc);
			fgetc(rsc);
		}
		caption[len] = '\0';
		if (code >= numLanguages)
			code = 0;
		fprintf(spec, "%s=%s\n", langs[code], caption);
	}
	uint8_t hidden = fgetc(rsc);
	uint8_t embeddability = fgetc(rsc);
	uint8_t newfile = fgetc(rsc);
	/*uint8_t launch =*/ fgetc(rsc);

	char mbmfile[PATH_MAX];
	sprintf(mbmfile, "%saif.mbm", dir);
	FILE* mbm = fopen(mbmfile, "rb");
	if (mbm) {
		fprintf(spec, "mbmfile=%s\n", mbmfile);
		fclose(mbm);
	}
	fprintf(spec, "hidden=%d\n", hidden);
	fprintf(spec, "embeddability=%d\n", embeddability);
	fprintf(spec, "newfile=%d\n", newfile);
	
	fclose(spec);
	fclose(rsc);

	char command[3*PATH_MAX];
	sprintf(command, "genaif -u %#x %s %sout.aif", uid, specfile, dir);
	int ret = system(command);
	if (ret) {
		printf("%s\n", command);
		printf("genaif returned %d\n", ret);
	}
	unlink(specfile);
	return ret;
}

