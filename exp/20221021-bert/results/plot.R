# data = read.delim('out.csv')
data = read.csv(file = 'plot_data.csv')
cols <-c("azure", "blanchedalmond", "azure4", "coral1")
pdf("Cost.pdf")
x <- barplot(data$Cost..., ylim=c(0,4), col=cols,
	 cex.lab=2, cex.axis=2)
y <-as.matrix(data$Cost)




pdf("Time.pdf")
x <- barplot(data$Time.s., ylim=c(0,6500), col=cols,
	  cex.lab=1.2, cex.axis=1.2)
y <-as.matrix(data$Time.s.)
