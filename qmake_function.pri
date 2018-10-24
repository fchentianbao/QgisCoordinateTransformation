#created by GPT tangyong 2013.0416

#defineReplace(qtDependedLibrary) {
#   unset(LIBRARY_NAME)
#   LIBRARY_NAME = $$1
#   contains(TEMPLATE, .*app):CONFIG(debug, debug|release) {
#      !debug_and_release|build_pass {
#          mac:RET = $$member(LIBRARY_NAME, 0)_debug
#              else:win32:RET = $$member(LIBRARY_NAME, 0)d
#      }
#   }
#   isEmpty(RET):RET = $$LIBRARY_NAME
#   return($$RET)
#}

defineReplace(qtDependedLibrary) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
              win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}
