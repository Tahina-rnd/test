import { api } from './api';
import type { LoginCredentials, AuthResponse, User } from '@/types';

export const authService = {
  async login(credentials: LoginCredentials): Promise<AuthResponse> {
    const response = await api.post<AuthResponse>('/auth/login', credentials);
    api.setToken(response.token);
    return response;
  },

  async logout(): Promise<void> {
    try {
      await api.post('/auth/logout');
    } finally {
      api.setToken(null);
    }
  },

  async getCurrentUser(): Promise<User> {
    return api.get<User>('/auth/me');
  },

  async refreshToken(): Promise<AuthResponse> {
    const response = await api.post<AuthResponse>('/auth/refresh');
    api.setToken(response.token);
    return response;
  },

  isAuthenticated(): boolean {
    return !!api.getToken();
  },
};
