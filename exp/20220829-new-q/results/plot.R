# data = read.delim('out.csv')
data = read.csv(file = 'out-1.csv')
data = data[data$Type != "GET",]
data$Type = as.factor(data$Type)
data = data[order(as.numeric(data$Type)),]

pdf("q1.pdf")
with(data, plot(Cost, Total.Time,
				pch=as.numeric(Type), col=as.numeric(Type)), ylim=c(10, 20))
with(data, legend('topleft', legend=levels(Type), 
				pch=unique(as.numeric(Type)), col=unique(as.numeric(Type))))
# with(data, text(Total.Cost, Total.Time, Chunk.Size, adj=c(0.2,-0.2)))

