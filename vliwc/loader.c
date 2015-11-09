#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "exit_codes.h"
#include "globals.h"
#include "ecoff.h"

static FILE *f;

void load(char *file_name) {
  int i;
  struct ecoff_filehdr file_hdr;
  struct ecoff_aouthdr aout_hdr;
  struct ecoff_scnhdr section_hdr;

  if ((f = fopen(file_name, "r")) == NULL)
    fatal("load, fopen", SYS_CALL_ERR);

  /* load the exec file header */
  if (fread(&file_hdr, sizeof(file_hdr), 1, f) != 1)
    fatal("load, fread, file_hdr", SYS_CALL_ERR);

  /* set the endianess */
  switch (file_hdr.f_magic) {
  case ECOFF_EB_MAGIC:
    big_endian = 1;
    break;
  case ECOFF_EL_MAGIC:
    big_endian = 0;
    break;
  default:
    fprintf(stderr, "Bad magic number in file header\n");
    exit(INTERF_ERR);
  }

  /* read the aout header */
  if (fread(&aout_hdr, sizeof(aout_hdr), 1, f) != 1)
    fatal("load, fread, aout_hdr", SYS_CALL_ERR);

  /* get the entry point. we're interested in it because it may define
     a new basic block */
  entry_point = aout_hdr.entry;
  /* go to the start of the first section */
  if (fseek(f, sizeof(file_hdr) + file_hdr.f_opthdr, SEEK_SET) == -1)
    fatal("load, fseek", SYS_CALL_ERR);

  for (i = 0; i < file_hdr.f_nscns; i++) {
    /* read the section header */
    if (fread(&section_hdr, sizeof(section_hdr), 1, f) != 1)
      fatal("load, fread, section_hdr", SYS_CALL_ERR);
    /* here we're interested only in the text segment */
    if (section_hdr.s_flags != ECOFF_STYP_TEXT)
      continue;

    text_start = section_hdr.s_vaddr;
    if (section_hdr.s_size == 0) {
      fprintf(stderr, "The program has no text segment\n");
      exit(INTERF_ERR);
    }

    if (text_size != 0) {
      fprintf(stderr, "The program has more than one text segment\n");
      exit(INTERF_ERR);
    }

    text_size = section_hdr.s_size;

    if (fseek(f, section_hdr.s_scnptr, SEEK_SET) == -1)
      fatal("load, fseek", SYS_CALL_ERR);
    if ((text_segment = (unsigned char*)calloc(section_hdr.s_size,
					       sizeof(unsigned char))) == NULL)
      fatal("load, malloc", SYS_CALL_ERR);
    if (fread(text_segment, section_hdr.s_size, 1, f) != 1)
      fatal("load, fread", SYS_CALL_ERR);
  }

  if (fclose(f) == -1)
    fatal("load, fclose", SYS_CALL_ERR);
}
