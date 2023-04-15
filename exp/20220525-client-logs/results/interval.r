library(plyr)
args = commandArgs(trailingOnly = TRUE)
df <- read.csv(file = args[1], header = TRUE, sep = ",")

nrow <- nrow(df)

print(nrow)

e <- max(df$ServerReceiveTime) + 1000
# # process df1
# sent <- df1$start[df1$start == df1$end] /1000

pdf(args[2])
plot(NULL, xaxt="n", xlim=c(0, e / 1000), ylim=c(0, nrow), xlab="Time (s)", ylab="query")
axis(side=1, at=c(seq(from=0, to=e / 1000, by=1)))
# axis(side=2, at=c(seq(from=0, to=nrow, by=200)))
y <- 0

for (i in 1:nrow(df)) {
	start = df$AcquireTime[i] /1000
	end = (df$ServerReceiveTime[i]) /1000
	lines(x=c(start, end), y=c(y, y))
	y = y + 1
}


# c <- count(sent)


# pre_x <- 0
# pre_y <- 0
# acc <- 0
# for (i in 1:nrow(c)) {
# 	t = c$x[i]
# 	acc = c$freq[i] + acc
# 	lines(x=c(pre_x, t), y=c(pre_y, acc), col='green')
# 	pre_x = t
# 	pre_y = acc
# }

# #process df2


# for (i in 1:nrow(df2)) {
# 	start = df2$start[i] / 1000
# 	end = df2$end[i] / 1000
# 	lines(x=c(start, end), y=c(y,y), col='blue')
# 	y = y + 1
# }

dev.off()