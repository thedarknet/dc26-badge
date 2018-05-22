stm32:
  if you update the version of flatbuffers you need to ensure that both esp and stm32 directories are updated at the same time.  Would have been nice to have them both point to the same directory but as I wanted to support windows builds there isn't (at least I could find one that worked well, outside of the linux env for windows) a true soft link so instead I copied to both directories.

esp: flatbuffers:
  if you update the version of flatbuffers you need to copy the include/flatbuffers/include directory to the flatbuffers directory

  then copy src/idl_parser.cpp (and src/idl_gen_text.cpp to this directory
