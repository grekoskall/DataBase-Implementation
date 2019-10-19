#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "heap_file.h"


#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

HP_ErrorCode HP_Init() {
  //insert code here

  return HP_OK;
}

HP_ErrorCode HP_CreateFile(const char *filename) {
  //insert code here
  CALL_BF(BF_CreateFile(filename));
  int fd;
  char* data;
  char type = 'h';
  int records = 0;
  BF_Block *block;
  BF_Block_Init(&block);
  CALL_BF(BF_OpenFile(filename, &fd));
  CALL_BF(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  memcpy(data, &type, sizeof(char));
  memcpy(data+sizeof(char), &records, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  CALL_BF(BF_CloseFile(fd));

  return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc){
  //insert code here
  int fd;
  BF_Block *block;
  BF_Block_Init(&block);

  char* data;
  char type;
  CALL_BF(BF_OpenFile(fileName, &fd));
  CALL_BF(BF_GetBlock(fd, 0, block));
  data = BF_Block_GetData(block);
  memcpy(&type, data, sizeof(char));
  if (type == 'h'){
	//printf("The file is a heap file.\n");	 
  } else {
	printf("Error: the file is not a heap file.\n");
  	return HP_ERROR;
  }
  CALL_BF(BF_UnpinBlock(block));
  *fileDesc = fd;
  memcpy(fileDesc, &fd, sizeof(int));
  
  //CALL_BF(BF_CloseFile(fd));
  return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc) {
  //insert code here
  CALL_BF(BF_CloseFile(fileDesc));
  return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record) {
  //insert code here
  BF_Block *block;
  BF_Block_Init(&block);
  Record *data;
  char *data2;
  int max_recordNum = BF_BLOCK_SIZE/sizeof(Record);
  int recordsNum, blocksNum, position;

  CALL_BF(BF_GetBlockCounter(fileDesc, &blocksNum));

  CALL_BF(BF_GetBlock(fileDesc, 0, block));
  data2 = BF_Block_GetData(block);
  memcpy( &recordsNum, data2+sizeof(char), sizeof(int));

  position = recordsNum%max_recordNum;
  //printf("Records : %d, Position: %d.\n", recordsNum, position);
  recordsNum += 1;
  memcpy(data2+sizeof(char), &recordsNum, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));

  if(position == 0){
  	CALL_BF(BF_AllocateBlock(fileDesc, block));
	data = (Record*)BF_Block_GetData(block);
	memcpy(data, &record, sizeof(Record));
  } else { 
  	CALL_BF(BF_GetBlock(fileDesc, blocksNum-1, block));
  	data = (Record*)BF_Block_GetData(block);
	memcpy(data+position, &record, sizeof(Record));
  }
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block)); 
  return HP_OK;
}



HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *attrName, void* value) {
  //insert code here
  BF_Block *block;
  BF_Block_Init(&block);
  char *data;
  Record *data2;
  int blocksNum, recordsNum;
  int max_recordNum = BF_BLOCK_SIZE/sizeof(Record);
  CALL_BF(BF_GetBlockCounter(fileDesc, &blocksNum));
  CALL_BF(BF_GetBlock(fileDesc, 0, block));
  data = BF_Block_GetData(block);
  memcpy(&recordsNum, data+sizeof(char), sizeof(int));
  CALL_BF(BF_UnpinBlock(block));
  if ( strcmp(attrName, "id") == 0){
  	int *id, blockNum = 0, position;
	id = value;	
	for(int i=0; i < recordsNum; i++){
		position = i%max_recordNum;
		if ( position == 0 ){
			blockNum += 1;
		}
		//printf("Block: %d, Position: %d\n", &blockNum, position);
		CALL_BF(BF_GetBlock(fileDesc, blockNum, block));
		data2 = (Record*)BF_Block_GetData(block);

		if ( data2[position].id == *id){
		printf("%d,\"%s\",\"%s\",\"%s\"\n", data2[position].id, data2[position].name, data2[position].surname, data2[position].city);
		}
		CALL_BF(BF_UnpinBlock(block));
	}
  } else if ( strcmp(attrName, "name") == 0){
	  int blockNum = 0, position;
	  char *name;
	  name = value;

	  for( int i = 0; i < recordsNum; i++){
	  	position = i%max_recordNum;
		if ( position == 0 ){
			blockNum += 1;
		}

		CALL_BF(BF_GetBlock(fileDesc, blockNum, block));
		data2 = (Record*)BF_Block_GetData(block);

		if ( strcmp(data2[position].name, name) == 0 ){
			printf("%d,\"%s\",\"%s\",\"%s\"\n", data2[position].id, data2[position].name, data2[position].surname, data2[position].city);
		}
		CALL_BF(BF_UnpinBlock(block));
	  }
  
  } else if ( strcmp(attrName, "surname") == 0){
	  int blockNum = 0, position;
	  char *surname;
	  surname = value;
	  
	  for( int i = 0; i < recordsNum; i++){
	  	position = i%max_recordNum;
		if ( position == 0 ){
			blockNum += 1;
		}

		CALL_BF(BF_GetBlock(fileDesc, blockNum, block));
		data2 = (Record*)BF_Block_GetData(block);

		if ( strcmp(data2[position].surname, surname) == 0) {
			printf("%d,\"%s\",\"%s\",\"%s\"\n", data2[position].id, data2[position].name, data2[position].surname, data2[position].city);
		}
		CALL_BF(BF_UnpinBlock(block));
	  }
  
  } else if ( strcmp(attrName, "city") == 0){
  	  int blockNum = 0, position;
	  char *city;
	  city = value;

	  for ( int i = 0; i < recordsNum; i++){
	  	position = i%max_recordNum;
		if ( position == 0) {
			blockNum += 1;
		}

		CALL_BF(BF_GetBlock(fileDesc, blockNum, block));
		data2 = (Record*)BF_Block_GetData(block);
		
		if ( strcmp(data2[position].city, city) == 0) {
			printf("%d,\"%s\",\"%s\",\"%s\"\n", data2[position].id, data2[position].name, data2[position].surname, data2[position].city);
		}
		CALL_BF(BF_UnpinBlock(block));
	  }
  } else {
  	printf("Error: incorrect attribute.\n");
	return HP_ERROR;
  }
  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  //insert code here
  int max_recordNum = BF_BLOCK_SIZE/sizeof(Record);
  int position = rowId%max_recordNum;
  int blockNum = rowId/max_recordNum+1;
  int recs;
  BF_Block *block;
  BF_Block_Init(&block);
  char *dt;

  CALL_BF(BF_GetBlock(fileDesc, 0, block));
  dt = BF_Block_GetData(block);
  memcpy( &recs, dt+sizeof(char), sizeof(int));
  //printf("%d, %d\n", rowId, recs);
  if ( rowId > recs ){
  	printf("Error: incorrect rowId\n");
	return HP_ERROR;
  }
  CALL_BF(BF_UnpinBlock(block));
  
  Record *data;

  CALL_BF(BF_GetBlock(fileDesc, blockNum, block));
  data = (Record*)BF_Block_GetData(block);
  memcpy(record, data+position, sizeof(Record));
  CALL_BF(BF_UnpinBlock(block));

  return HP_OK;
}
