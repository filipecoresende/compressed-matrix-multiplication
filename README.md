Esta versão usa unordered map e priority queue do C++. 

Após gerar o executável "repair" com o make, você pode comprimir um arquivo texto com:

./repair -c dataset/nome_do_arquivo.txt

Isto gerará um arquivo binário. Se o arquivo binário for, por exemplo, nome_do_arquivo.re16, você pode descomprimí-lo com:

./repair -d dataset/nome_do_arquivo.re16

Isso gerará um arquivo texto com nome de decompressedString.txt.