# data = read.delim('out.csv')
data = read.csv(file = 'out-q2.csv')
# data = data[data$Type != "Multi-fetch",]
data$Type = as.factor(data$Type)
data = data[order(as.numeric(data$Type)),]

pdf("q2.pdf")
with(data, plot(Total.Cost, Total.Time,
				pch=as.numeric(Type), col=as.numeric(Type)))
with(data, legend('topright', legend=levels(Type), 
				pch=unique(as.numeric(Type)), col=unique(as.numeric(Type))))
with(data, text(Total.Cost, Total.Time, Chunk.Size, adj=c(0.2,-0.2)))

