data = read.csv(file = 'out.csv')
# data = data[data$Chunk.Size == "4",]
# data$Type = as.factor(data$Type)
# data = data[order(as.numeric(data$Type)),]
read_data_time = aggregate(data$Read.Data.Time.s., by=list(data$Chunk.Size), FUN=mean)
y1 = read_data_time[,2]
write_back_time = aggregate(data$Write.Back.Time.s., by=list(data$Chunk.Size), FUN=mean)
y2 = write_back_time[,2]
x = write_back_time[,1]

read_reg <- lm(y1~x)
print(read_reg)

write_reg <- lm(y2~x)
print(read_reg)

pdf("read_data_time.pdf")
with(data, plot(Chunk.Size, Read.Data.Time.s., log='xy'))
abline(read_reg,untf=TRUE)

pdf("write_back_time.pdf")
with(data, plot(Chunk.Size, Write.Back.Time.s., log='xy'))
abline(write_reg,untf=TRUE)
# with(data, plot(Chunk.Size, Write.Back,
# 				pch=as.numeric(Type), col=as.numeric(Type)))
# with(data, legend('topright', legend=levels(Type), 
# 				pch=unique(as.numeric(Type)), col=unique(as.numeric(Type))))
# with(data, text(Total.Cost, Total.Time, Chunk.Size, adj=c(0.2,-0.2)))

intercept <- coef(write_reg)["(Intercept)"] + coef(read_reg)["(Intercept)"]
slope <- coef(write_reg)["x"] + coef(read_reg)["x"]

pdf("billed_time.pdf")
with(data, plot(Chunk.Size, Billed.Duration.s., log='xy'))
abline(intercept, slope,untf=TRUE)