# data = read.delim('out.csv')
data = read.csv(file = 'out.csv')
data$Type = as.factor(data$Type)
data = data[order(as.numeric(data$Type)),]

with(data, plot(Total.Cost, Total.Time,
				pch=as.numeric(Type), col=as.numeric(Type)))
with(data, legend('topright', legend=levels(Type), 
				pch=unique(as.numeric(Type)), col=unique(as.numeric(Type))))
with(data, text(Total.Cost, Total.Time, Chunk.Size, adj=c(0.5,-0.5)))

