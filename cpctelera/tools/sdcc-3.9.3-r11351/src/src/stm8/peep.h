bool stm8notUsed(const char *what, lineNode *endPl, lineNode *head);
bool stm8notUsedFrom(const char *what, const char *label, lineNode *head);
bool stm8canAssign (const char *dst, const char *src, const char *exotic);
int stm8instructionSize(lineNode *node);

