/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/04 12:32:36 by miokrako          #+#    #+#             */
/*   Updated: 2025/03/31 10:19:24 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFT_H
# define LIBFT_H
# include <stdlib.h>
# include <stddef.h>
# include <unistd.h>

int				ft_isalnum(int c);
int				ft_isalpha(int c);
int				ft_isascii(int c);
int				ft_isdigit(int c);
int				ft_isprint(int c);
size_t			ft_strlen(const char *str);
int				ft_strncmp(const char *s1, const char *s2, size_t n);
int				ft_tolower(int c);
int				ft_toupper(int c);
void			*ft_memset(void *s, int c, size_t n);
void			ft_bzero(void *s, size_t n);
size_t			ft_strlcat(char *dst, const char *src, size_t size);
void			*ft_memcpy(void *dest, const void *src, size_t n);
void			*ft_memmove(void *dest, const void *src, size_t n);
char			*ft_strchr(const char *s, int c);
void			*ft_memcpy(void *dest, const void *src, size_t n);
char			*ft_strrchr(const char *s, int c);
size_t			ft_strlcpy(char *dst, const char *src, size_t size);
void			*ft_memchr(const void *ptr, int value, size_t num);
int				ft_memcmp(const void *ptr1, const void *ptr2, size_t num);
char			*ft_strnstr(const char *str, const char *find, size_t len);
int				ft_atoi(const char *str);
char			*ft_strdup(const char *src);
char			*ft_substr(char const *s, unsigned int start, size_t len);
void			*ft_calloc(size_t nmemb, size_t size);
char			*ft_strjoin(char const *s1, char const *s2);
char			*ft_strtrim(char const *s1, char const *set);
char			*ft_itoa(int n);
char			**ft_split(char const *s, char c);
char			*ft_strmapi(char const *s, char (*f)(unsigned int, char));
void			ft_striteri(char *s, void (*f)(unsigned int, char*));
void			ft_putchar_fd(char c, int fd);
void			ft_putstr_fd(char *s, int fd);
void			ft_putendl_fd(char *s, int fd);
void			ft_putnbr_fd(int n, int fd);

#endif
