library(ggplot2)
library(ggpubr)
library(ggpattern)
library(dplyr)

text_size <- 45
title_size <-50
point_size <-25
cap_size <-70



myTheme <-theme(axis.text.x = element_text(size=text_size,color="black",margin=margin(t=15)),
  axis.text.y = element_text(size=text_size, color="black"),
  axis.title.x=element_text(size=title_size, color="black", face="bold",margin = margin(t = 15, r = 0, b = 0, l = 0)),
  axis.title.y=element_text(size=title_size,color="black", face="bold")) + 
  theme(legend.text = element_text(size = 50), 
                                  legend.title = element_text(size = 60, face="bold"), 
                                  legend.key.size = unit(4, 'cm'),
                                  # legend.position = c(20, 15),
                                  legend.box="vertical",
        legend.margin = margin(b=5, unit='cm')) +
  theme(panel.grid.major.y = element_line(color="black", size=0.75,linetype=2),
        panel.background = element_rect(fill='transparent'),
        plot.background = element_rect(fill='transparent', color=NA), 
        axis.line.y = element_line(arrow = grid::arrow(angle=15,
                                                       ends = "last", type="closed"),size=2),
        axis.line.x = element_line(arrow = grid::arrow(angle=15,
                                                       ends = "last", type="closed"),size=2))


data <- read.csv(file = '../data/data.csv')
data<- data[data$Query=="Row-based",]
print(data)
p1<-ggplot(data, aes(Platform, Time)) + 
  geom_bar_pattern(stat = "identity", 
                   pattern = c("none","stripe","none","stripe","stripe","none","stripe","stripe"),
                   pattern_angle = c(0,45,0,135,45,0,135,45),
                   pattern_density = 0.05,
                   pattern_spacing = 0.02,
                   pattern_fill = 'black',
                   color="black",
                   aes(fill = Operator), 
                   position=position_dodge2(preserve = "single")) +
  scale_fill_manual(" ", 
                    labels = c("Get", "Lambda", "Range"),
                    values = c("Get" = "black", "Range"="grey45", "Lambda"="white")) +
  guides(fill = guide_legend(title="Operator",
                              override.aes = 
                               list(
                                 pattern = c("none", "stripe", "stripe"),
                                 pattern_spacing = .1,
                                 pattern_density = .05,
                                 pattern_angle = c(0, 135, 45)
                                 )
                             )) +
myTheme +
labs(caption="(a) Time") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))+
xlab('Platform') +
ylab('Time(s)') +
scale_y_continuous(limits=c(0,10), breaks=c(0,2,4,6,8,10), expand=c(0,0))

p2<-ggplot(data, aes(Platform, Cost)) + 
  geom_bar_pattern(stat = "identity", 
                   pattern = c("none","stripe","none","stripe","stripe","none","stripe","stripe"),
                   pattern_angle = c(0,45,0,135,45,0,135,45),
                   pattern_density = 0.05,
                   pattern_spacing = 0.02,
                   pattern_fill = 'black',
                   color="black",
                   aes(fill = Operator), 
                   position=position_dodge2(preserve = "single")) +
  scale_fill_manual(" ", 
                    labels = c("Get", "Lambda", "Range"),
                    values = c("Get" = "black", "Range"="grey45", "Lambda"="white")) +
  guides(fill = guide_legend(title="Operator",
                              override.aes = 
                               list(
                                 pattern = c("none", "stripe", "stripe"),
                                 pattern_spacing = .02,
                                 pattern_density = .05,
                                 pattern_angle = c(0, 135, 45)
                                 )
                             )) +
myTheme +
labs(caption="(b) Cost") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))+
xlab('Platform') +
ylab('Cost($)') +
scale_y_continuous(limits=c(0,0.06), breaks=c(0,0.01,0.02,0.03,0.04,0.05,0.06), expand=c(0,0))

p <- ggarrange(p1, NULL, p2, nrow=1, common.legend = TRUE, widths = c(1,0.1,1))

ggsave(file="moti-row.eps", plot=p, width=45, height=20, device="eps")