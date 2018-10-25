#pragma once

void IdeCommon(int drive, int blockNumber, int count);
void IdeRead(int drive, int blockNumber, int count, char * buffer);
void IdeWrite(int drive, int blockNumber, int count, char * buffer);