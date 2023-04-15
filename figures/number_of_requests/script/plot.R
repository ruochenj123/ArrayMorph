library(ggplot2)
library(ggpubr)
text_size <- 30
title_size <-40
line_size <- 2.5
point_size <-7
cap_size <-50
myTheme <- theme(panel.grid.major.y = element_line(color="black", size=0.75,linetype=2))
myTheme <- myTheme + theme(legend.text = element_text(size = text_size), 
                                  legend.title = element_text(size = title_size, face="bold"), 
                                  legend.key.size = unit(4, 'cm'),
                                  legend.position = c(20, 10),
                                  legend.box="vertical",
                                  legend.margin=margin(b=4, unit="cm")) +
theme(panel.grid.major.y = element_line(color="black", size=0.75,linetype=2),
        panel.background = element_rect(fill='transparent'),
        plot.background = element_rect(fill='transparent', color=NA), 
        axis.line.y = element_line(arrow = grid::arrow(angle=15,
                                                       ends = "last", type="closed"),size=2),
        axis.line.x = element_line(arrow = grid::arrow(angle=15,
                                                       ends = "last", type="closed"),size=2))





df = read.csv(file = '../data/requests.csv')
# print(df)
p1 <- ggplot(df, aes(x=requests, y=throughput, group=Platform)) + 
geom_line(aes(linetype=Platform), size=line_size) + 
geom_point(aes(shape=Platform),size=point_size)

p1 <- p1 +
theme(axis.text.x = element_text(size=text_size, color="black" ,face="bold"),
	axis.text.y = element_text(size=text_size, color="black", face="bold"),
	axis.title.x=element_text(size=title_size, face="bold", margin = margin(t = 15, r = 0, b = 0, l = 0)),
	axis.title.y=element_text(size=title_size,face="bold")) +
xlab('# of requests') +
ylab('Throughput (MB/s)') +
labs(caption="(a) Varying number of requests") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))


p1 <- p1 + 
scale_x_continuous(trans='log2',breaks=c(0, 8,32,128,512,2048,8192,32768,131072,524288))  + 
scale_y_continuous(trans='log2',limits=c(1,128), breaks=c(2, 8,32,128))

p1 <- p1 +
myTheme


df1 = read.csv(file = '../data/chunk.csv')
# print(df)
p2 <- ggplot(df1, aes(x=chunk, y=throughput, group=Platform)) + 
geom_line(aes(linetype=Platform), size=line_size) + 
geom_point(aes(shape=Platform),size=point_size)

p2 <- p2 +
theme(axis.text.x = element_text(size=text_size, color="black" ,face="bold"),
	axis.text.y = element_text(size=text_size, color="black" ,face="bold"),
	axis.title.x=element_text(size=title_size, face="bold", margin = margin(t = 15, r = 0, b = 0, l = 0)),
	axis.title.y=element_text(size=title_size, face="bold")) +
xlab('Chunk Size (MB)') +
ylab('Throughput (MB/s)') +
labs(caption="(b) Varying chunk size") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))

p2 <- p2 + 
scale_x_continuous(trans='log2',breaks=c(1,2,4,8,16,32))  + 
scale_y_continuous(trans='log2',limits=c(1,128), breaks=c(2, 8,32,128))

p2 <- p2 +
myTheme
p <- ggarrange(p1, p2, ncol=2, nrow=1, common.legend = TRUE)

ggsave(file="throughput.eps", plot=p, width=30, height=15, device="eps")