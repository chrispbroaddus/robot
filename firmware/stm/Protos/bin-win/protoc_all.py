import os

try:
  os.mkdir('gen')
except OSError:
  if not os.path.isdir('gen'):
    raise
    
for root,dir,files in os.walk('.'):
  for file in files:
    if file.endswith('.proto'):
      cmd = ('..\\Middlewares\\nanopb-0.3.8\\bin-win\\protoc.exe --nanopb_out=gen --proto_path=. %s' % os.path.join(root,file))
      os.system(cmd)
