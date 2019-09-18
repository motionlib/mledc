
HERE = File.split(__FILE__)[0]

def load_data
  src = File.join(HERE, "totalized.txt" )
  o = {}
  nc=[]
  len=[]
  File.open( src ) do |f|
    f.each do |line|
      m=%r!(\d+)\, (\d+)\, medc\:ok\/ng\:(\d+)/(\d+)  f32\:ok\/ng\:(\d+)/(\d+)!.match(line)
      next unless m
      mok,mng, fok,fng = m[3,4].map(&:to_i)
      nc << m[1].to_i
      len << m[2].to_i
      o[m[1]+"/"+m[2]] = { mledc:[mok,mng], f32:[fok,fng] }
    end
  end
  [nc.minmax, len.minmax, o]
end

def col(n)
  return "#0f0" if n==0
  v=n+50
  r = v
  g = 255-v
  b = 0
  "#%02x%02x%02x" % [ r, g, b ]
end