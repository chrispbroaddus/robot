
function write_matrix_for_cpp(A, type, precision, name)

rows = size(A, 1);
cols = size(A, 2);

if(rows == 1 && cols == 1)
    fprintf('%s %s = ', type, name);
    fprintf(precision, A(1,1));
    fprintf(';\n');
else
    fprintf('%s %s[] = {\n', type, name);
    
    for i = 1:rows
        for j = 1:cols
            fprintf(precision, A(i,j));
            if(j < cols)
                fprintf(', ');
            elseif i < rows
                fprintf(',');
            end
        end
    
        if(i < rows)
            fprintf('\n');
        end
    end
    fprintf('\n};\n');
end
