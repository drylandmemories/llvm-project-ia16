set(LLVM_LINK_COMPONENTS
  BinaryFormat
  Object
  Support
  )

add_llvm_tool(llvm-ia16-mz
  llvm-ia16-mz.cpp
  )
