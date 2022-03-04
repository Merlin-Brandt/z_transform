%: init.m5
%: c.m5

%: {::function_definition} -> {}
void main(char *argv, int argc)
{
    struct {
        int a, b;
        char **f[10];
    } test;
}
EOF

   