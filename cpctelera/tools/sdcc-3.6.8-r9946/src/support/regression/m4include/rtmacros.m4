divert(-1)
#
# macros borrowed from m4 examples
#

# from forloop3.m4:
# forloop_arg(from, to, macro) - invoke MACRO(value) for
#   each value between FROM and TO, without define overhead
define(`forloop_arg', `ifelse(eval(`($1) <= ($2)'), `1',
  `_forloop(`$1', eval(`$2'), `$3(', `)')')')
# forloop(var, from, to, stmt) - refactored to share code
define(`forloop', `ifelse(eval(`($2) <= ($3)'), `1',
  `pushdef(`$1')_forloop(eval(`$2'), eval(`$3'),
    `define(`$1',', `)$4')popdef(`$1')')')
define(`_forloop',
  `$3`$1'$4`'ifelse(`$1', `$2', `',
    `$0(incr(`$1'), `$2', `$3', `$4')')')

# from quote.m4:
# quote(args) - convert args to single-quoted string
define(`quote', `ifelse(`$#', `0', `', ``$*'')')
# dquote(args) - convert args to quoted list of quoted strings
define(`dquote', ``$@'')
# dquote_elt(args) - convert args to list of double-quoted strings
define(`dquote_elt', `ifelse(`$#', `0', `', `$#', `1', ```$1''',
                             ```$1'',$0(shift($@))')')

# from foreachq4.m4:
# foreachq(x, `item_1, item_2, ..., item_n', stmt)
#   quoted list, version based on forloop
define(`foreachq',
`ifelse(`$2', `', `', `_$0(`$1', `$3', $2)')')
define(`_foreachq',
`pushdef(`$1', forloop(`$1', `3', `$#',
  `$0_(`1', `2', indir(`$1'))')`popdef(
    `$1')')indir(`$1', $@)')
define(`_foreachq_',
``define(`$$1', `$$3')$$2`''')

# from foreach2.m4:
# foreach(x, (item_1, item_2, ..., item_n), stmt)
#   parenthesized list, improved version
define(`foreach', `pushdef(`$1')_$0(`$1',
  (dquote(dquote_elt$2)), `$3')popdef(`$1')')
define(`_arg1', `$1')
define(`_foreach', `ifelse(`$2', `(`')', `',
  `define(`$1', _arg1$2)$3`'$0(`$1', (dquote(shift$2)), `$3')')')

divert(0)dnl
