#ifndef __OSOCKADDR_H__
#define __OSOCKADDR_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 4.3BSD sockaddr structure (used by talk protocol).  */
struct osockaddr {
	u_short	sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

#ifdef __cplusplus
}
#endif


#endif /* __OSOCKADDR_H__ */
