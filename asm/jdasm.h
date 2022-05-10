#ifndef __JDASM_H__
#define __JDASM_H__

#include "JD1304.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <map>

void assemble(const char *filename, const char *output);

#endif