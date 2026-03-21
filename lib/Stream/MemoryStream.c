/**
 * @file MemoryStream.c
 * @brief MemoryStream の実装
 * @author atohs
 * @date 2024/11/05
 */
#include "Stream/MemoryStream.h"
#include "utilities.h"
#include <stdlib.h>
#include <string.h>

MemoryStreamReturnCode MemoryStream_Init(MemoryStream *self) {
  if (UNLIKELY(!self)) {
    return MEMORY_STREAM_INVALID_ARG;
  }
  CLEAR(self);
  self->stream = open_memstream(&self->buffer, &self->position);
  if (UNLIKELY(!self->stream)) {
    return MEMORY_STREAM_OPEN_FAILED;
  }
  fflush(self->stream); /* buffer ポインタと position を初期化する */
  return MEMORY_STREAM_SUCCESS;
}

void MemoryStream_Destroy(MemoryStream *self) {
  if (UNLIKELY(!self)) {
    return;
  }
  if (self->stream) {
    fclose(self->stream);
  }
  free(self->buffer);
  CLEAR(self);
}

ssize_t MemoryStream_Write(MemoryStream *self, const void *data, size_t size) {
  if (UNLIKELY(!self)) {
    return -1;
  }
  size_t written = fwrite(data, size, 1, self->stream);
  if (written != 1) {
    return -1;
  }
  fflush(self->stream);
  off_t pos = ftello(self->stream);
  if (pos != -1 && (size_t)pos > self->size) {
    self->size = (size_t)pos;
  }
  return (ssize_t)size;
}

size_t MemoryStream_Read(MemoryStream *self, void *buffer, size_t size) {
  if (UNLIKELY(!self)) {
    return 0;
  }
  fflush(self->stream);
  off_t pos = ftello(self->stream);
  if (UNLIKELY(pos < 0)) {
    return 0;
  }
  size_t cur = (size_t)pos;
  if (cur >= self->size) {
    return 0;
  }
  size_t available = self->size - cur;
  size_t toRead = size < available ? size : available;
  memcpy(buffer, self->buffer + cur, toRead);
  fseeko(self->stream, (off_t)toRead, SEEK_CUR);
  return toRead;
}

MemoryStreamReturnCode MemoryStream_Seek(MemoryStream *self,
                                         off_t offset,
                                         int whence) {
  if (UNLIKELY(!self)) {
    return MEMORY_STREAM_INVALID_ARG;
  }
  if (fseeko(self->stream, offset, whence) != 0) {
    return MEMORY_STREAM_SEEK_ERROR;
  }
  return MEMORY_STREAM_SUCCESS;
}

const void *MemoryStream_RefBuffer(MemoryStream *self) {
  if (UNLIKELY(!self)) {
    return NULL;
  }
  fflush(self->stream);
  return self->buffer;
}

size_t MemoryStream_GetSize(MemoryStream *self) {
  if (UNLIKELY(!self)) {
    return 0;
  }
  return self->size;
}
